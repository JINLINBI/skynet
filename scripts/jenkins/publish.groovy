#!/usr/bin/env groovy

pipeline {
    agent {
		node { label 'master' }
	}
    stages {
        stage('Clean') {
            steps {
                echo 'start cleanup!'
                sh '''
                 mkdir -p aces
                '''
            }
        }
        stage('CheckOut-Git') {
            steps {
                echo 'start git checkout!'
                sh '''
                    if [ ! -d "server_remix03" ];then
                        git clone http://192.168.50.178/backend/server_remix03.git server_remix03
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
                    rsync -vzrtopg --progress --exclude=bin/cfg/local --exclude=bin/log  bin emina@43.136.106.111:/data/game/server_remix03/
                '''
               }
        }
        
    }
    
}