#!/usr/bin/env groovy
// To get a more sensible workspace directory name

def buildConfiguration = "Debug"
def configurationList = """Debug
RelWithDebInfo"""
properties([disableConcurrentBuilds(),
              parameters([
                choice(choices: configurationList, name: "buildConfiguration")
              ])
           ])

buildConfiguration = params.buildConfiguration

@NonCPS
def getWorkspace = { buildType ->
  def directory_name = pwd().tokenize("\\").last()

  pwd().replace("%2F", "_") + buildType
}

def getBranch = {
    env.BRANCH_NAME.tokenize("/").last()
}

@NonCPS
def post_stage_failure(job, stage, error, url){
  slackSend channel:"eax-builds", color: "danger", message: "Build ${job} failed in stage ${stage} with ${error}, see: <${url}|results>"
}
@NonCPS
def post_build_finished(job,url){
  slackSend channel:"eax-builds", color: "good", message: "Build ${job} finished successfully: see <${url}|result>"
}

def test_python(){ node ('ubuntu-16.04'){
    timestamps {
      try{
        sh "cp protobuf/network_pb2.py python/veles/"
        dir('python') {
          sh "./run_tests.sh"
          junit allowEmptyResults: true, keepLongStdio: true, testResults: 'res**.xml'
        }
      } catch (error) {
        post_stage_failure(env.JOB_NAME, "python-ubuntu1604",error,env.BUILD_URL)
        throw error
      }
    }
  }
}

def builders = [:]

builders['mingw32'] = { node('windows') {
    ws(getWorkspace("")){
      timestamps {
        try {
          stage('windows minigw') {
            deleteDir()
            checkout scm
            def branch = getBranch()
            withEnv(["PATH+QT=${env.QT}\\Tools\\mingw530_32\\bin;${env.QT}\\5.7\\mingw53_32\\bin",
                     "PATH+CMAKE=${env.CMAKE}\\bin",
                     "GOOGLETEST_DIR=${tool 'googletest'}"]) {
              bat(script: "rd /s /q build_mingw", returnStatus: true)
              bat script: "md build_mingw", returnStatus: true
              dir('build_mingw') {
                bat script: "cmake -DPROTOBUF_LIBRARY_DEBUG=\"${tool 'protobuf_mingw'}debug_mingw/libprotobufd.dll.a\" -DPROTOBUF_LIBRARY=\"${tool 'protobuf_mingw'}release_mingw/libprotobuf.dll.a\"\
                -DPROTOBUF_DLL_DIR=${tool 'protobuf'}dll_mingw -DProtobuf_INCLUDE_DIR=${tool 'protobuf'}src\
                -DProtobuf_PROTOC_EXECUTABLE=${tool 'protobuf'}release_mingw\\protoc.exe\
                -DCMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH% -DGOOGLETEST_SRC_PATH=%GOOGLETEST_DIR% -G \"MinGW Makefiles\" -DCMAKE_BUILD_TYPE=${buildConfiguration} .."
                bat script: "cmake --build . --config ${buildConfiguration}"
                bat script: "cpack -D CPACK_PACKAGE_FILE_NAME=veles-mingw -G ZIP -C ${buildConfiguration}"
              }
            }
            junit allowEmptyResults: true, keepLongStdio: true, testResults: '**/results.xml'
            step([$class: 'ArtifactArchiver', artifacts: 'build_mingw/veles-mingw.zip', fingerprint: true])
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
            withEnv(["PATH+QT=${env.QT}\\Tools\\${version}\\bin;${env.QT}\\5.7\\${version}\\bin",
                     "PATH+CMAKE=${env.CMAKE}\\bin",
                     "CMAKE_PREFIX_PATH+QT=${env.QT}\\5.7\\${version}",
                     "GOOGLETEST_DIR=${tool 'googletest'}"
                     ]){
              if(version.contains("64")){
                generator = "${generator} Win64"
              }
              bat(script: "rd /s /q build_${version}", returnStatus: true)
              bat script: "md build_${version}", returnStatus: true
              dir ("build_${version}") {
                bat script: "cmake -DPROTOBUF_LIBRARY_DEBUG=${tool 'protobuf'}debug64\\Debug\\libprotobufd.lib -DPROTOBUF_LIBRARY=${tool 'protobuf'}release64\\RelWithDebInfo\\libprotobuf.lib\
                  -DPROTOBUF_DLL_DIR=${tool 'protobuf'}dll64 -DProtobuf_PROTOC_EXECUTABLE=${tool 'protobuf'}release64\\RelWithDebInfo\\protoc.exe\
                  -DPROTOBUF_SRC_ROOT_FOLDER=${tool 'protobuf'}\
                  -DCMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH% -DGOOGLETEST_SRC_PATH=%GOOGLETEST_DIR% -G \"${generator}\" ..\\"
                bat script: "cmake --build . --config ${buildConfiguration} > error_and_warnings.txt"
                bat script: "type error_and_warnings.txt"
                bat script: "cpack -D CPACK_PACKAGE_FILE_NAME=veles-${version} -G ZIP -C ${buildConfiguration}"
              }
              junit allowEmptyResults: true, keepLongStdio: true, testResults: '**/results.xml'
              step([$class: 'ArtifactArchiver', artifacts: "build_${version}/veles-${version}.zip", fingerprint: true])
              step([$class: 'WarningsPublisher', canComputeNew: false, canResolveRelativePaths: false, defaultEncoding: '',
                    excludePattern: '', healthy: '', includePattern: '', messagesPattern: '',
                    parserConfigurations: [[parserName: 'MSBuild', pattern: "build_${version}/error_and_warnings.txt"]], unHealthy: ''])
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
            withEnv(["PATH+QT=${env.QT}\\Tools\\${version}\\bin;${env.QT}\\5.7\\${version}\\bin",
                     "PATH+CMAKE=${env.CMAKE}\\bin",
                     "CMAKE_PREFIX_PATH+QT=${env.QT}\\5.7\\${version}",
                     "GOOGLETEST_DIR=${tool 'googletest'}"]){
              if(version.contains("64")){
                generator = "${generator} Win64"
              }
              bat(script: "rd /s /q build_${version}", returnStatus: true)
              bat script: "md build_${version}", returnStatus: true
              dir ("build_${version}") {
                bat script: "cmake -DPROTOBUF_LIBRARY_DEBUG=${tool 'protobuf'}debug\\Debug\\libprotobufd.lib -DPROTOBUF_LIBRARY=${tool 'protobuf'}release\\RelWithDebInfo\\libprotobuf.lib\
                  -DPROTOBUF_DLL_DIR=${tool 'protobuf'}dll -DProtobuf_PROTOC_EXECUTABLE=${tool 'protobuf'}release\\RelWithDebInfo\\protoc.exe\
                  -DPROTOBUF_SRC_ROOT_FOLDER=${tool 'protobuf'}\
                  -DCMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH% -DGOOGLETEST_SRC_PATH=%GOOGLETEST_DIR% -G \"${generator}\" ..\\"
                bat script: "cmake --build . --config ${buildConfiguration}"
                bat script: "cpack -D CPACK_PACKAGE_FILE_NAME=veles-${version} -G ZIP -C ${buildConfiguration}"
              }
              junit allowEmptyResults: true, keepLongStdio: true, testResults: '**/results.xml'
              step([$class: 'ArtifactArchiver', artifacts: "build_${version}/veles-${version}.zip", fingerprint: true])
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
          sh 'rm -Rf *'
          checkout scm
          def branch = getBranch()
          // finding protobuf doesn't like if there is older version installed and requires some additional variables
          sh """cmake -DPROTOBUF_INCLUDE_DIR=\"${tool 'protobuf'}\"/src -DPROTOBUF_PROTOC_EXECUTABLE=\"${tool 'protobuf'}\"/release/protoc\
             -DPROTOBUF_LIBRARY_DEBUG=\"${tool 'protobuf'}\"/debug/libprotobufd.so -DPROTOBUF_LIBRARY=\"${tool 'protobuf'}\"/release/libprotobuf.so\
             -DGOOGLETEST_SRC_PATH=\"${tool 'googletest'}\" -DCMAKE_BUILD_TYPE=${buildConfiguration} CMakeLists.txt"""
          sh "cmake --build . --config ${buildConfiguration} 2>&1 | tee error_and_warnings.txt"
          sh "cpack -D CPACK_PACKAGE_FILE_NAME=veles-ubuntu1604 -G ZIP -C ${buildConfiguration}"
          junit allowEmptyResults: true, keepLongStdio: true, testResults: '**/results.xml'
          step([$class: 'ArtifactArchiver', artifacts: '*.zip', fingerprint: true])
          step([$class: 'WarningsPublisher', canComputeNew: false, canResolveRelativePaths: false, defaultEncoding: '',
          excludePattern: '', healthy: '', includePattern: '', messagesPattern: '',
          parserConfigurations: [[parserName: 'GNU Make + GNU C Compiler (gcc)', pattern: 'error_and_warnings.txt']], unHealthy: ''])
        }
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
        stage('macOS 10.12 w/ Apple Clang 8.0.0') {
          checkout scm
          sh 'rm -Rf *'
          checkout scm
          def branch = getBranch()
          sh "cmake CMakeLists.txt -G \"Xcode\" -DCMAKE_PREFIX_PATH=$QT/5.7/clang_64 -DGOOGLETEST_SRC_PATH=\"${tool 'googletest'}\"\
              -DProtobuf_INCLUDE_DIR=\"${tool 'protobuf'}\"src -DProtobuf_PROTOC_EXECUTABLE=\"${tool 'protobuf'}\"release/RelWithDebInfo/protoc\
              -DPROTOBUF_LIBRARY_DEBUG=\"${tool 'protobuf'}\"debug/Debug/libprotobufd.dylib -DPROTOBUF_LIBRARY=\"${tool 'protobuf'}\"release/RelWithDebInfo/libprotobuf.dylib"
          sh "cmake --build . --config ${buildConfiguration} 2>&1 | tee error_and_warnings.txt"
          sh "cpack -D CPACK_PACKAGE_FILE_NAME=veles-osx -G ZIP -C ${buildConfiguration}"
          junit allowEmptyResults: true, keepLongStdio: true, testResults: '**/results.xml'
          step([$class: 'ArtifactArchiver', artifacts: '*.zip', fingerprint: true])
          step([$class: 'WarningsPublisher', canComputeNew: false, canResolveRelativePaths: false, defaultEncoding: '',
          excludePattern: '', healthy: '', includePattern: '', messagesPattern: '',
          parserConfigurations: [[parserName: 'Clang (LLVM based)', pattern: 'error_and_warnings.txt']], unHealthy: ''])
        }
      } catch (error) {
        post_stage_failure(env.JOB_NAME, "macOS-10.12",error,env.BUILD_URL)
        throw error
      }
    }
  }
}

//parallel builders


parallel(builders)
test_python()
stage('Build finished') {
	node(){
  	post_build_finished(env.JOB_NAME, env.BUILD_URL)
  }
}

