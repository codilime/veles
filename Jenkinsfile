#!/usr/bin/env groovy
// To get a more sensible workspace directory name

def buildConfiguration = "Debug"
def configurationList = """Debug
RelWithDebInfo"""
def platforms = 'all'
def platformList = """all
ubuntu-16.04
mingw32
msvc2015
msvc2015_64
macosx"""
properties([disableConcurrentBuilds(),
             parameters([
               choice(choices: configurationList, name: "buildConfiguration"),
               choice(choices: platformList , description: 'Platform to build', name: 'Platform')])
           ])

buildConfiguration = params.buildConfiguration
platforms = params.Platform

@NonCPS
def getWorkspace = { buildType ->
  def directory_name = pwd().tokenize("\\").last()

  pwd().replace("%2F", "_") + buildType
}

def getBranch = {
    env.BRANCH_NAME.tokenize("/").last()
}



def getVersion() {
    def revision = readFile("REVISION").trim()
    def shortsha = isUnix() ? sh(script: 'git rev-parse --short HEAD', returnStdout: true).trim() : bat(script: 'git rev-parse --short HEAD', returnStdout: true).trim()
    return "${revision}-${shortsha}"
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
        try{
        deleteDir()
        checkout scm
        def branch = getBranch()
        withEnv(["PATH+QT=${env.QT}\\Tools\\mingw530_32\\bin;${env.QT}\\5.7\\mingw53_32\\bin",
                 "PATH+CMAKE=${env.CMAKE}\\bin",
                 "GTEST_DIR=${tool 'gtest'}"]) {
          bat(script: "rd /s /q build_mingw", returnStatus: true)
          bat script: "md build_mingw", returnStatus: true
          dir('build_mingw') {
            bat script: "cmake -DCMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH% -DGTEST_SRC_PATH=%GTEST_DIR% -G \"MinGW Makefiles\" -DCMAKE_BUILD_TYPE=${buildConfiguration} .."
            bat script: "cmake --build . --config ${buildConfiguration}"
            bat script: "cpack -D CPACK_PACKAGE_FILE_NAME=veles-mingw -G ZIP -C ${buildConfiguration}"
          }
        }
        junit allowEmptyResults: true, keepLongStdio: true, testResults: '**/results.xml'
        step([$class: 'ArtifactArchiver', artifacts: 'build_mingw/veles-mingw.zip', fingerprint: true])
        }
        catch (error){
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
          deleteDir()
          checkout scm
          def branch = getBranch()
          def generator = "Visual Studio 14 2015"
          withEnv(["PATH+QT=${env.QT}\\Tools\\${version}\\bin;${env.QT}\\5.7\\${version}\\bin",
                   "PATH+CMAKE=${env.CMAKE}\\bin",
                   "CMAKE_PREFIX_PATH+QT=${env.QT}\\5.7\\${version}",
                   "GTEST_DIR=${tool 'gtest'}"
                   ]){
            if(version.contains("64")){
              generator = "${generator} Win64"
            }
            bat(script: "rd /s /q build_${version}", returnStatus: true)
            bat script: "md build_${version}", returnStatus: true
            dir ("build_${version}") {
              bat script: "cmake -DCMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH% -DGTEST_SRC_PATH=%GTEST_DIR% -G \"${generator}\" ..\\"
              bat script: "cmake --build . --config ${buildConfiguration}"
              bat script: "cpack -D CPACK_PACKAGE_FILE_NAME=veles-${version} -G ZIP -C ${buildConfiguration}"
            }
            junit allowEmptyResults: true, keepLongStdio: true, testResults: '**/results.xml'
            step([$class: 'ArtifactArchiver', artifacts: "build_${version}/veles-${version}.zip", fingerprint: true])
          }
        }  catch (error) {
          post_stage_failure(env.JOB_NAME, "windows-msvc2015-x64",error,env.BUILD_URL)
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
        try{
          deleteDir()
          checkout scm
          def branch = getBranch()
          def generator = "Visual Studio 14 2015"
          withEnv(["PATH+QT=${env.QT}\\Tools\\${version}\\bin;${env.QT}\\5.7\\${version}\\bin",
                   "PATH+CMAKE=${env.CMAKE}\\bin",
                   "CMAKE_PREFIX_PATH+QT=${env.QT}\\5.7\\${version}",
                   "GTEST_DIR=${tool 'gtest'}"]){
            if(version.contains("64")){
              generator = "${generator} Win64"
            }
            bat(script: "rd /s /q build_${version}", returnStatus: true)
            bat script: "md build_${version}", returnStatus: true
            dir ("build_${version}") {
              bat script: "cmake -DCMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH% -DGTEST_SRC_PATH=%GTEST_DIR% -G \"${generator}\" ..\\"
              bat script: "cmake --build . --config ${buildConfiguration}"
              bat script: "cpack -D CPACK_PACKAGE_FILE_NAME=veles-${version} -G ZIP -C ${buildConfiguration}"
            }
            junit allowEmptyResults: true, keepLongStdio: true, testResults: '**/results.xml'
            step([$class: 'ArtifactArchiver', artifacts: "build_${version}/veles-${version}.zip", fingerprint: true])
          }
        }catch (error) {
          post_stage_failure(env.JOB_NAME, "windows-msvc2015-x86",error,env.BUILD_URL)
          throw error
        }
      }
    }
  }
}


builders['ubuntu-16.04'] = { node ('ubuntu-16.04'){
    timestamps {
      try{
        checkout scm
        sh 'rm -Rf *'
        checkout scm
        def version = getVersion()
        def branch = getBranch()
        sh "cmake -DGTEST_SRC_PATH=\"${tool 'gtest'}\" -DCMAKE_BUILD_TYPE=${buildConfiguration} CMakeLists.txt"
        sh 'cmake --build . --config ${buildConfiguration}'
        sh 'cpack -D CPACK_PACKAGE_FILE_NAME=veles-ubuntu1604 -G ZIP -C ${buildConfiguration}'
        junit allowEmptyResults: true, keepLongStdio: true, testResults: '**/results.xml'
        step([$class: 'ArtifactArchiver', artifacts: '*.zip', fingerprint: true])
      } catch (error) {
        post_stage_failure(env.JOB_NAME, "ubuntu-16.04-amd64",error,env.BUILD_URL)
        throw error
      }
    }
  }
}

builders['macosx'] = { node ('macosx'){
    timestamps {
      try{
        checkout scm
        sh 'rm -Rf *'
        checkout scm
        def version = getVersion()
        def branch = getBranch()
        sh "cmake CMakeLists.txt -G \"Xcode\" -DCMAKE_PREFIX_PATH=$QT/5.7/clang_64 -DGTEST_SRC_PATH=\"${tool 'gtest'}\""
        sh 'cmake --build . --config ${buildConfiguration}'
        sh 'cpack -D CPACK_PACKAGE_FILE_NAME=veles-osx -G ZIP -C ${buildConfiguration}'
        junit allowEmptyResults: true, keepLongStdio: true, testResults: '**/results.xml'
        step([$class: 'ArtifactArchiver', artifacts: '*.zip', fingerprint: true])
      } catch (error) {
        post_stage_failure(env.JOB_NAME, "macOS-10.12",error,env.BUILD_URL)
        throw error
      }
    }
  }
}

//parallel builders

def build_platform = { platform ->
  if (platforms == 'all' || platforms == platform){
    builders[platform]()
  }else{
    echo "Skipping platform ${platform}"
  }
}

stage 'linux ubuntu 16.04'
build_platform('ubuntu-16.04')


stage 'windows mingw'
build_platform('mingw32')



stage 'windows msvc2015 x64'
build_platform('msvc2015_64')

stage 'windows msvc2015 x86'
build_platform('msvc2015')

stage 'macOS 10.12 w/ Apple Clang 8.0.0'
build_platform('macosx')

node(){
  post_build_finished(env.JOB_NAME, env.BUILD_URL)
}

