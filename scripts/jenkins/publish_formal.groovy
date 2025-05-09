
#!/usr/bin/env groovy

pipeline {
    agent {
		node { label 'master' }
	}
    stages {
        stage('Check') {
            steps {
                script {
                    // 二次确认
                    def userInput = input(
                        id: 'deployConfirmation', message: 'Are you sure you want to deploy?', parameters: [
                        [$class: 'BooleanParameterDefinition', defaultValue: false, description: 'Please confirm.', name: 'confirm']
                    ])
 
                    // 检查用户是否确认
                    if (userInput.confirm) {
                        echo 'Deploying application...'
                        // 这里执行部署代码
                    } else {
                        error "Deployment cancelled :("
                    }
                }
            }
        }
        stage('Clean') {
            steps {
                echo 'start cleanup!'
                sh '''
                 mkdir -p remix03
                '''
            }
        }
        stage('CheckOut-Git') {
            steps {
                echo 'start git checkout!'
                sh '''
                    if [ ! -d "server_remix03" ];then
                        git clone git@192.168.50.178:backend/remix03/server_remix03.git server_remix03
                    else
                        cd server_remix03; git reset --hard  HEAD; git checkout main; git pull
                    fi
                '''
            }
        }
        stage('Sync Server Bin') {
            steps {
                sh '''
                    cd server_remix03
                    rm -rf bin/cfg/*
                    rsync -vzrtopg --progress --exclude=bin/cfg/local --exclude=bin/log  bin emina@42.193.108.113:/data/game/server_remix03/
                '''
               }
        }
        
    }
    
}