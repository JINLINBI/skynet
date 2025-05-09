#!/usr/bin/env groovy

pipeline {
    agent {
		node { label 'master' }
	}
    stages {
        stage('tool-Git') {
            steps {
                echo 'start git checkout!'
                sh '''
                    if [ ! -d "config_tools" ];then
                        git clone git@192.168.1.59:Common/config_tools.git config_tools
                    fi
                    
                    rm -rf /var/lib/jenkins/.local/share/CarpPyTools/
                    cd config_tools; mkdir -p data; 
                    git reset --hard  HEAD; git pull; git checkout remix03;
                '''
            }
        }
        stage('CheckOut-SVN') {
            steps {
                echo 'start svn checkout!'
                dir("remix03") {
                checkout([$class: 'SubversionSCM', additionalCredentials: [], excludedCommitMessages: '', excludedRegions: '', excludedRevprop: '', excludedUsers: '', filterChangelog: false, ignoreDirPropChanges: false, includedRegions: '', locations: [[cancelProcessOnExternalsFail: true, credentialsId: 'svn-ci', depthOption: 'infinity', ignoreExternalsOption: true, local: '.', remote: 'svn://192.168.1.45/remix03/DesktopPet']], quietOperation: true, workspaceUpdater: [$class: 'UpdateUpdater']])

                }
            }
        }
        stage('Convert table') {
            steps {
                sh '''
                    cd remix03/assets/config
                    python3 ../../../config_tools/tool.py config . 
                    cd ../../..
                '''
            }
        }
        stage('commit table data') {
            steps {
                sh '''
                    cd server_remix03
                    git checkout -B ${BRANCH_NAME#origin/} ${BRANCH_NAME}

                    if ls ../config_tools/data/*.lua 1> /dev/null 2>&1; then
                        cp -f ../config_tools/data/*.lua bin/table
                    fi

                    git add -u bin/table
                    git config --global user.email "ci@emina.com"
                    git config --global user.name "ci"
                    git commit -m 'table sync' || {
                        echo "配置没有变化，空提交"
                        echo "no thing committed"
                        exit 0
                    }
                    git push origin ${BRANCH_NAME#origin/}
                   '''
            }
        }
        stage('finish') {
            steps {
                sh '''
                
                '''
            }
        }
        
    }
    
}
