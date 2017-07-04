#!/usr/bin/env groovy
// TODO: Get a more sensible workspace directory name
/**
 * This Jenkins file uses custom Warning parsers for clang-tidy output.
 * Jenkins / Manage Jenkins / Configure System
 * In section "Compiler Warnings" three parsers needs to be configured:
 * clang-tidy-gcc, clang-tidy-clang, clang-tidy-msvc
 * Regular Expression: (?m)^(.+?):(\d+):(?:\d+:)?(?:\{\d:-\}+)?(?:.*) (warning|error): (.*) \[([^\]]*)\]
 * Mapping script:
import hudson.plugins.warnings.parser.Warning
import hudson.plugins.analysis.util.model.Priority

String fileName = matcher.group(1)
int lineNumber = Integer.parseInt(matcher.group(2))
String category = matcher.group(5)
String message = matcher.group(4)
Priority priority = Priority.NORMAL

if (category.contains('error')) priority = Priority.HIGH

return new Warning(fileName, lineNumber, "Clang-tidy", category, message, priority)

 */


def getBranch = {
    env.BRANCH_NAME.tokenize("/").last()
}

def buildConfiguration = "Debug"
def configurationList = """Debug
RelWithDebInfo"""
def buildsToKeep = getBranch() == 'master' ? "30" : "5"

properties([
  disableConcurrentBuilds(),
  parameters([
    choice(choices: configurationList, name: "buildConfiguration")
  ]),
  [$class: 'jenkins.model.BuildDiscarderProperty',
    strategy: [$class: 'LogRotator',
      numToKeepStr: buildsToKeep,
      artifactNumToKeepStr: buildsToKeep
    ]
  ]
])

buildConfiguration = params.buildConfiguration

@NonCPS
def getWorkspace = { buildType ->
  def directory_name = pwd().tokenize("\\").last()

  pwd().replace("%2F", "_") + buildType
}

@NonCPS
def post_stage_failure(job, stage, build_id, url){
  httpRequest url: "http://hubot.codilime.com:32768/hubot/jenkins-notify?room=eax-builds", httpMode: "POST", contentType: "APPLICATION_JSON", requestBody: "{\"build\": {\"phase\": \"FINISHED\", \"status\": \"FAILURE\", \"number\": ${build_id}, \"full_url\": \"${url}\"}, \"name\": \"${job} stage: ${stage} *FAIL* :red_circle: \" }"
}
@NonCPS
def post_build_finished(job, build_id, url){
  httpRequest url: "http://hubot.codilime.com:32768/hubot/jenkins-notify?room=eax-builds", httpMode: "POST", contentType: "APPLICATION_JSON", requestBody: "{\"build\": {\"phase\": \"FINISHED\", \"status\": \"SUCCESS\", \"number\": ${build_id}, \"full_url\": \"${url}\"}, \"name\": \"${job} :green_heart:\" }"
}

def builders = [:]

builders['msvc2015_64'] = { node('windows'){
    def version = 'msvc2015_64'
    ws(getWorkspace("")){
      timestamps {
        stage('checkout windows msvc2015 x64') {
          try {
            checkout scm
          }  catch (error) {
            post_stage_failure(env.JOB_NAME, "windows checkout", env.BUILD_ID, env.BUILD_URL)
            throw error
          }
        }

        parallel(
          "build windows msvc2015 x64": {
            stage('windows msvc2015 x64') {
              try {
                def branch = getBranch()
                def generator = "Visual Studio 14 2015"
                def vcredist_binary = "vcredist_x64.exe"
                def cpack_generator = "ZIP"
                withEnv(["PATH+QT=${env.QT}\\Tools\\${version}\\bin;${env.QT}\\5.7\\${version}\\bin",
                         "PATH+CMAKE=${env.CMAKE}\\bin",
                         "CMAKE_PREFIX_PATH+QT=${env.QT}\\5.7\\${version}",
                         "GOOGLETEST_DIR=${tool 'googletest'}",
                         "EMBED_PYTHON_ARCHIVE_PATH=${tool 'embed-python-64'}",
                         "VCINSTALLDIR=${env.VS}\\VC\\"
                         ]){
                  if(version.contains("64")){
                    generator = "${generator} Win64"
                  }
                  if (!buildConfiguration.equals("Debug")) {
                    cpack_generator = "${cpack_generator};NSIS"
                  }
                  bat(script: "rd /s /q build_${version}", returnStatus: true)
                  bat script: "md build_${version}", returnStatus: true
                  dir ("build_${version}") {
                    bat script: "cmake -DCMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH% -DGOOGLETEST_SRC_PATH=%GOOGLETEST_DIR% -DEMBED_PYTHON_ARCHIVE_PATH=%EMBED_PYTHON_ARCHIVE_PATH% -DVCREDIST_BINARY=\"${vcredist_binary}\" -DOPENSSL_DLL_DIR=${tool 'openssl-64'} -G \"${generator}\" ..\\"
                    bat script: "cmake --build . --config ${buildConfiguration} -- /maxcpucount:8 > errors_and_warnings.txt"
                    bat script: "type errors_and_warnings.txt"
                    bat script: "cmake --build . --target lint --config ${buildConfiguration} -- /maxcpucount:8 > lint_errors_and_warnings.txt"
                    bat script: "type lint_errors_and_warnings.txt"
                    bat script: "cpack -D CPACK_PACKAGE_FILE_NAME=veles-${version} -G \"${cpack_generator}\" -C ${buildConfiguration}"
                  }
                  junit allowEmptyResults: true, keepLongStdio: true, testResults: '**/results.xml'
                  step([$class: 'ArtifactArchiver', artifacts: "build_${version}/veles-${version}.*", fingerprint: true])
                  step([$class: 'WarningsPublisher',
                    canComputeNew: false,
                    canResolveRelativePaths: false,
                    defaultEncoding: '',
                    excludePattern: '',
                    healthy: '',
                    includePattern: '',
                    messagesPattern: '',
                    parserConfigurations: [
                      [parserName: 'MSBuild', pattern: "build_${version}/errors_and_warnings.txt"],
                      [parserName: 'clang-tidy-msvc', pattern: "build_${version}/lint_errors_and_warnings.txt"]
                    ],
                    unHealthy: ''
                  ])

                  step([$class: 'WarningsPublisher', canComputeNew: false, canResolveRelativePaths: false, defaultEncoding: '',
                        excludePattern: '', healthy: '', includePattern: '', messagesPattern: '',
                        parserConfigurations: [[parserName: 'MSBuild', pattern: "build_${version}/errors_and_warnings.txt"]], unHealthy: ''])
                  bat(script: "rd /s /q build_${version}", returnStatus: true)
                }
              }  catch (error) {
                post_stage_failure(env.JOB_NAME, "windows-msvc2015-x64", env.BUILD_ID, env.BUILD_URL)
                bat script: "type build_${version}\\*errors_and_warnings.txt"
                throw error
              }
            }
          },
          "python tests - Windows": {
            stage('python tests - Windows') {
              try {
                dir('python') {
                  bat "run_tests.bat"
                  junit allowEmptyResults: true, keepLongStdio: true, testResults: 'res**.xml'
                }
              } catch (error) {
                post_stage_failure(env.JOB_NAME, "python-windows", env.BUILD_ID, env.BUILD_URL)
                throw error
              }
            }
          }
        )
      }
    }
  }
}

builders['msvc2015'] = {node('windows'){
    def version = "msvc2015"
    ws(getWorkspace("")){
      timestamps {
        try {
          stage('windows msvc2015 x86') {
            checkout scm
            def branch = getBranch()
            def generator = "Visual Studio 14 2015"
            def vcredist_binary = "vcredist_x86.exe"
            def cpack_generator = "ZIP"
            withEnv(["PATH+QT=${env.QT}\\Tools\\${version}\\bin;${env.QT}\\5.7\\${version}\\bin",
                     "PATH+CMAKE=${env.CMAKE}\\bin",
                     "CMAKE_PREFIX_PATH+QT=${env.QT}\\5.7\\${version}",
                     "GOOGLETEST_DIR=${tool 'googletest'}",
                     "EMBED_PYTHON_ARCHIVE_PATH=${tool 'embed-python-32'}",
                     "VCINSTALLDIR=${env.VS}\\VC\\"]){
              if(version.contains("64")){
                generator = "${generator} Win64"
              }
              if (!buildConfiguration.equals("Debug")) {
                cpack_generator = "${cpack_generator};NSIS"
              }
              bat(script: "rd /s /q build_${version}", returnStatus: true)
              bat script: "md build_${version}", returnStatus: true
              dir ("build_${version}") {
                bat script: "cmake -DCMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH% -DGOOGLETEST_SRC_PATH=%GOOGLETEST_DIR% -DEMBED_PYTHON_ARCHIVE_PATH=%EMBED_PYTHON_ARCHIVE_PATH% -DVCREDIST_BINARY=\"${vcredist_binary}\" -DOPENSSL_DLL_DIR=${tool 'openssl-32'} -G \"${generator}\" ..\\"
                bat script: "cmake --build . --config ${buildConfiguration} -- /maxcpucount:8"
                bat script: "cpack -D CPACK_PACKAGE_FILE_NAME=veles-${version} -G \"${cpack_generator}\" -C ${buildConfiguration}"
              }
              junit allowEmptyResults: true, keepLongStdio: true, testResults: '**/results.xml'
              step([$class: 'ArtifactArchiver', artifacts: "build_${version}/veles-${version}.*", fingerprint: true])
              bat(script: "rd /s /q build_${version}", returnStatus: true)
            }
          }
        } catch (error) {
          post_stage_failure(env.JOB_NAME, "windows-msvc2015-x86", env.BUILD_ID, env.BUILD_URL)
          throw error
        }
      }
    }
  }
}


builders['ubuntu-16.04'] = { node ('ubuntu-16.04'){
    timestamps {
      stage('checkout ubuntu 16.04') {
        try {
          checkout scm
          sh 'rm -Rf build'
        } catch (error) {
          post_stage_failure(env.JOB_NAME, "Ubuntu checkout", env.BUILD_ID, env.BUILD_URL)
          throw error
        }
      }
      parallel(
        "build ubuntu 16.04": {
          stage('ubuntu 16.04') {
            try {
              dir ("build") {
                def branch = getBranch()
                sh """cmake -DGOOGLETEST_SRC_PATH=\"${tool 'googletest'}\" -DCMAKE_BUILD_TYPE=${buildConfiguration} .."""
                sh """#!/bin/bash -ex
                  set -o pipefail
                  cmake --build . --config ${buildConfiguration} -- -j3 2>&1 | tee errors_and_warnings.txt
                """
                sh """#!/bin/bash -ex
                  set -o pipefail
                  cmake --build . --target lint --config ${buildConfiguration} -- -j3 2>&1 | tee lint_errors_and_warnings.txt
                """

                sh "cpack -D CPACK_PACKAGE_FILE_NAME=veles-ubuntu1604 -G \"ZIP;DEB\" -C ${buildConfiguration}"
                junit allowEmptyResults: true, keepLongStdio: true, testResults: '**/results.xml'
                step([$class: 'ArtifactArchiver', artifacts: 'veles-ubuntu1604.*', fingerprint: true])
                step([$class: 'WarningsPublisher',
                  canComputeNew: false,
                  canResolveRelativePaths: false,
                  defaultEncoding: '',
                  excludePattern: '',
                  healthy: '',
                  includePattern: '',
                  messagesPattern: '',
                  parserConfigurations: [
                    [parserName: 'GNU Make + GNU C Compiler (gcc)', pattern: 'errors_and_warnings.txt'],
                    [parserName: 'clang-tidy-gcc', pattern: 'lint_errors_and_warnings.txt'],
                  ],
                  unHealthy: ''
                ])
              }
              sh 'rm -Rf build'
            } catch (error) {
              post_stage_failure(env.JOB_NAME, "ubuntu-16.04-amd64", env.BUILD_ID, env.BUILD_URL)
              throw error
            }
          }
        },
        "python tests - Linux ubuntu 16.04": {
          stage('python tests - Linux ubuntu 16.04') {
            try {
              dir('python') {
                sh "./run_tests.sh"
                junit allowEmptyResults: true, keepLongStdio: true, testResults: 'res**.xml'
              }
            } catch (error) {
              post_stage_failure(env.JOB_NAME, "python-ubuntu1604", env.BUILD_ID, env.BUILD_URL)
              throw error
            }
          }
        }
      )
    }
  }
}

builders['macosx'] = { node ('macosx'){
    timestamps {
      stage('checkout macOS') {
        try {
          checkout scm
          sh 'rm -Rf build'
        } catch (error) {
          post_stage_failure(env.JOB_NAME, "macOS checkout", env.BUILD_ID, env.BUILD_URL)
          throw error
        }
      }
      parallel(
        "build macOS 10.12 w/ Apple Clang 8.0.0": {
          stage('macOS 10.12 w/ Apple Clang 8.0.0') {
            try {
              dir ("build") {
                def branch = getBranch()
                sh "cmake .. -G \"Xcode\" -DCMAKE_PREFIX_PATH=$QT/5.7/clang_64 -DGOOGLETEST_SRC_PATH=\"${tool 'googletest'}\""
                sh """#!/bin/bash -ex
                  set -o pipefail
                  cmake --build . --config ${buildConfiguration} 2>&1 | tee errors_and_warnings.txt
                """
                sh """#!/bin/bash -ex
                  set -o pipefail
                  cmake --build . --target lint --config ${buildConfiguration} 2>&1  | tee lint_errors_and_warnings.txt
                """
                sh "cpack -D CPACK_PACKAGE_FILE_NAME=veles-osx -G \"DragNDrop;ZIP\" -C ${buildConfiguration}"
                junit allowEmptyResults: true, keepLongStdio: true, testResults: 'results.xml'
                step([$class: 'ArtifactArchiver', artifacts: 'veles-osx.*', fingerprint: true])
                step([$class: 'WarningsPublisher',
                  canComputeNew: false,
                  canResolveRelativePaths: false,
                  defaultEncoding: '',
                  excludePattern: '',
                  healthy: '',
                  includePattern: '',
                  messagesPattern: '',
                  parserConfigurations: [
                    [parserName: 'Clang (LLVM based)', pattern: 'errors_and_warnings.txt'],
                    [parserName: 'clang-tidy-clang', pattern: 'lint_errors_and_warnings.txt']
                  ],
                  unHealthy: ''
                ])
              }
              sh 'rm -Rf build'
            } catch (error) {
              post_stage_failure(env.JOB_NAME, "macOS-10.12", env.BUILD_ID, env.BUILD_URL)
              throw error
            }
          }
        },
        "python tests - macOS 10.12": {
          stage('python tests - macOS 10.12') {
            try {
              dir('python') {
                sh "./run_tests.sh"
                junit allowEmptyResults: true, keepLongStdio: true, testResults: 'res**.xml'
              }
            } catch (error) {
              post_stage_failure(env.JOB_NAME, "python-macosx", env.BUILD_ID, env.BUILD_URL)
              throw error
            }
          }
        }
      )
    }
  }
}

//parallel builders


parallel(builders)
node(){
	post_build_finished(env.JOB_NAME, env.BUILD_ID, env.BUILD_URL)
}

