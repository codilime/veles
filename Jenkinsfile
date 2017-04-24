#!/usr/bin/env groovy
// To get a more sensible workspace directory name

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
def post_stage_failure(job, stage, error, url){
  slackSend channel:"eax-builds", color: "danger", message: "Build ${job} failed in stage ${stage} with ${error}, see: <${url}|results>"
}
@NonCPS
def post_build_finished(job,url){
  slackSend channel:"eax-builds", color: "good", message: "Build ${job} finished successfully: see <${url}|result>"
}

def builders = [:]

builders['mingw32'] = { node('windows') {
    ws(getWorkspace("")){
      timestamps {
        try {
          stage('windows mingw') {
            deleteDir()
            checkout scm
            def branch = getBranch()
            def cpack_generator = "ZIP"
            withEnv(["PATH+QT=${env.QT}\\Tools\\mingw530_32\\bin;${env.QT}\\5.7\\mingw53_32\\bin",
                     "PATH+CMAKE=${env.CMAKE}\\bin",
                     "GOOGLETEST_DIR=${tool 'googletest'}",
                     "EMBED_PYTHON_ARCHIVE_PATH=${tool 'embed-python-32'}"]) {
              if (!buildConfiguration.equals("Debug")) {
                cpack_generator = "${cpack_generator};NSIS"
              }
              bat(script: "rd /s /q build_mingw", returnStatus: true)
              bat script: "md build_mingw", returnStatus: true
              dir('build_mingw') {
                bat script: "cmake -DCMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH% -DGOOGLETEST_SRC_PATH=%GOOGLETEST_DIR% -DEMBED_PYTHON_ARCHIVE_PATH=%EMBED_PYTHON_ARCHIVE_PATH% -G \"MinGW Makefiles\" -DCMAKE_BUILD_TYPE=${buildConfiguration} .."
                bat script: "cmake --build . --config ${buildConfiguration} -- -j3"
                bat script: "cpack -D CPACK_PACKAGE_FILE_NAME=veles-mingw -G \"${cpack_generator}\" -C ${buildConfiguration}"
              }
            }
            junit allowEmptyResults: true, keepLongStdio: true, testResults: '**/results.xml'
            step([$class: 'ArtifactArchiver', artifacts: 'build_mingw/veles-mingw.*', fingerprint: true])
            bat(script: "rd /s /q build_mingw", returnStatus: true)
          }
        } catch (error) {
          post_stage_failure(env.JOB_NAME, "windows-mingw32",error,env.BUILD_URL)
          throw error
        }
      }
    }
  }
}

builders['msvc2015_64'] = { node('windows'){
    def version = 'msvc2015_64'
    ws(getWorkspace("")){
      timestamps {
        try {
          stage('windows msvc2015 x64') {
            deleteDir()
            checkout scm
            def branch = getBranch()
            def generator = "Visual Studio 14 2015"
            def cpack_generator = "ZIP"
            withEnv(["PATH+QT=${env.QT}\\Tools\\${version}\\bin;${env.QT}\\5.7\\${version}\\bin",
                     "PATH+CMAKE=${env.CMAKE}\\bin",
                     "CMAKE_PREFIX_PATH+QT=${env.QT}\\5.7\\${version}",
                     "GOOGLETEST_DIR=${tool 'googletest'}",
                     "EMBED_PYTHON_ARCHIVE_PATH=${tool 'embed-python-64'}"
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
                bat script: "cmake -DCMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH% -DGOOGLETEST_SRC_PATH=%GOOGLETEST_DIR% -DEMBED_PYTHON_ARCHIVE_PATH=%EMBED_PYTHON_ARCHIVE_PATH% -G \"${generator}\" ..\\"
                bat script: "cmake --build . --config ${buildConfiguration} > error_and_warnings.txt"
                bat script: "type error_and_warnings.txt"
                bat script: "cpack -D CPACK_PACKAGE_FILE_NAME=veles-${version} -G \"${cpack_generator}\" -C ${buildConfiguration}"
              }
              junit allowEmptyResults: true, keepLongStdio: true, testResults: '**/results.xml'
              step([$class: 'ArtifactArchiver', artifacts: "build_${version}/veles-${version}.*", fingerprint: true])
              step([$class: 'WarningsPublisher', canComputeNew: false, canResolveRelativePaths: false, defaultEncoding: '',
                    excludePattern: '', healthy: '', includePattern: '', messagesPattern: '',
                    parserConfigurations: [[parserName: 'MSBuild', pattern: "build_${version}/error_and_warnings.txt"]], unHealthy: ''])
              bat(script: "rd /s /q build_${version}", returnStatus: true)
            }
          }
        }  catch (error) {
          post_stage_failure(env.JOB_NAME, "windows-msvc2015-x64",error,env.BUILD_URL)
          bat script: "type build_${version}\\error_and_warnings.txt"
          throw error
        }
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
            deleteDir()
            checkout scm
            def branch = getBranch()
            def generator = "Visual Studio 14 2015"
            def cpack_generator = "ZIP"
            withEnv(["PATH+QT=${env.QT}\\Tools\\${version}\\bin;${env.QT}\\5.7\\${version}\\bin",
                     "PATH+CMAKE=${env.CMAKE}\\bin",
                     "CMAKE_PREFIX_PATH+QT=${env.QT}\\5.7\\${version}",
                     "GOOGLETEST_DIR=${tool 'googletest'}",
                     "EMBED_PYTHON_ARCHIVE_PATH=${tool 'embed-python-32'}"]){
              if(version.contains("64")){
                generator = "${generator} Win64"
              }
              if (!buildConfiguration.equals("Debug")) {
                cpack_generator = "${cpack_generator};NSIS"
              }
              bat(script: "rd /s /q build_${version}", returnStatus: true)
              bat script: "md build_${version}", returnStatus: true
              dir ("build_${version}") {
                bat script: "cmake -DCMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH% -DGOOGLETEST_SRC_PATH=%GOOGLETEST_DIR% -DEMBED_PYTHON_ARCHIVE_PATH=%EMBED_PYTHON_ARCHIVE_PATH% -G \"${generator}\" ..\\"
                bat script: "cmake --build . --config ${buildConfiguration}"
                bat script: "cpack -D CPACK_PACKAGE_FILE_NAME=veles-${version} -G \"${cpack_generator}\" -C ${buildConfiguration}"
              }
              junit allowEmptyResults: true, keepLongStdio: true, testResults: '**/results.xml'
              step([$class: 'ArtifactArchiver', artifacts: "build_${version}/veles-${version}.*", fingerprint: true])
              bat(script: "rd /s /q build_${version}", returnStatus: true)
            }
          }
        } catch (error) {
          post_stage_failure(env.JOB_NAME, "windows-msvc2015-x86",error,env.BUILD_URL)
          throw error
        }
      }
    }
  }
}


builders['ubuntu-16.04'] = { node ('ubuntu-16.04'){
    timestamps {
      try {
        stage('ubuntu 16.04') {
          checkout scm
          sh 'rm -Rf build'
          dir ("build") {
            def branch = getBranch()
            sh """cmake -DGOOGLETEST_SRC_PATH=\"${tool 'googletest'}\" -DCMAKE_BUILD_TYPE=${buildConfiguration} .."""
            sh "cmake --build . --config ${buildConfiguration} -- -j3 > error_and_warnings.txt 2>&1"
            sh "cat error_and_warnings.txt"
            sh "cpack -D CPACK_PACKAGE_FILE_NAME=veles-ubuntu1604 -G \"ZIP;DEB\" -C ${buildConfiguration}"
            junit allowEmptyResults: true, keepLongStdio: true, testResults: '**/results.xml'
            step([$class: 'ArtifactArchiver', artifacts: 'veles-ubuntu1604.*', fingerprint: true])
            step([$class: 'WarningsPublisher', canComputeNew: false, canResolveRelativePaths: false, defaultEncoding: '',
            excludePattern: '', healthy: '', includePattern: '', messagesPattern: '',
            parserConfigurations: [[parserName: 'GNU Make + GNU C Compiler (gcc)', pattern: 'error_and_warnings.txt']], unHealthy: ''])
          }
          sh 'rm -Rf build'
        }
      } catch (error) {
        sh "cat build/error_and_warnings.txt"
        post_stage_failure(env.JOB_NAME, "ubuntu-16.04-amd64",error,env.BUILD_URL)
        throw error
      }
    }
  }
}

builders['macosx'] = { node ('macosx'){
    timestamps {
      try{
        stage('macOS 10.12 w/ Apple Clang 8.0.0') {
          checkout scm
          sh 'rm -Rf build'
          dir ("build") {
            def branch = getBranch()
            sh "cmake .. -G \"Xcode\" -DCMAKE_PREFIX_PATH=$QT/5.7/clang_64 -DGOOGLETEST_SRC_PATH=\"${tool 'googletest'}\""
            sh "cmake --build . --config ${buildConfiguration} > error_and_warnings.txt 2>&1"
            sh "cat error_and_warnings.txt"
            sh "cpack -D CPACK_PACKAGE_FILE_NAME=veles-osx -G \"DragNDrop;ZIP\" -C ${buildConfiguration}"
            junit allowEmptyResults: true, keepLongStdio: true, testResults: '**/results.xml'
            step([$class: 'ArtifactArchiver', artifacts: 'veles-osx.*', fingerprint: true])
            step([$class: 'WarningsPublisher', canComputeNew: false, canResolveRelativePaths: false, defaultEncoding: '',
            excludePattern: '', healthy: '', includePattern: '', messagesPattern: '',
            parserConfigurations: [[parserName: 'Clang (LLVM based)', pattern: 'error_and_warnings.txt']], unHealthy: ''])
          }
          sh 'rm -Rf build'
        }
      } catch (error) {
        sh "cat build/error_and_warnings.txt"
        post_stage_failure(env.JOB_NAME, "macOS-10.12",error,env.BUILD_URL)
        throw error
      }
    }
  }
}

builders['python-windows'] = { node ('windows'){    timestamps {
      try {
        stage('python tests - Windows 10') {
          checkout scm
          dir('python') {
            bat "run_tests.bat"
            junit allowEmptyResults: true, keepLongStdio: true, testResults: 'res**.xml'
          }
        }
      } catch (error) {
        post_stage_failure(env.JOB_NAME, "python-windows",error,env.BUILD_URL)
        throw error
      }
    }
  }
}

builders['python-ubuntu'] = { node ('ubuntu-16.04'){
    timestamps {
      try {
        stage('python tests - Linux ubuntu 16.04') {
          checkout scm
          dir('python') {
            sh "./run_tests.sh"
            junit allowEmptyResults: true, keepLongStdio: true, testResults: 'res**.xml'
          }
        }
      } catch (error) {
        post_stage_failure(env.JOB_NAME, "python-ubuntu1604",error,env.BUILD_URL)
        throw error
      }
    }
  }
}

builders['python-macosx'] = { node ('macosx'){
    timestamps {
      try {
        stage('python tests - macOS 10.12') {
          checkout scm
          dir('python') {
            sh "./run_tests.sh"
            junit allowEmptyResults: true, keepLongStdio: true, testResults: 'res**.xml'
          }
        }
      } catch (error) {
        post_stage_failure(env.JOB_NAME, "python-macosx",error,env.BUILD_URL)
        throw error
      }
    }
  }
}
//parallel builders


parallel(builders)
node(){
	post_build_finished(env.JOB_NAME, env.BUILD_URL)
}

