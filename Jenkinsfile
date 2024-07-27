pipeline 
/*
Sprite Utils

Requirements:

Maven is Installed

Java 21 is installed - variable JAVA_21_HOME is set

*/
{
    agent any
    environment {
    	AAA = 'aaa'
    }
    stages
    {
        stage('Build')
        {
            steps {
            
	            echo "*** Building ${env.JOB_NAME} ***"
    		    sh '''
    		        #!/bin/bash
    		        echo JOB_NAME=$JOB_NAME

    		        if [ -z "$JAVA_21_HOME" ]
                        then
                              echo "KO : Variable JAVA_21_HOME is empty. You fix this issue by adding this variable to configuration of Jenkins."
                              exit 1
                        else
                              echo "OK : Variable JAVA_21_HOME is NOT empty"
                        fi
                        export JAVA_HOME=$JAVA_21_HOME
                        case $BRANCH_NAME in

    		          master | deploy_prod)
                                mvn clean install
        		        ;;
    		        
      		          develop | jenkins | deploy_test)
        		        echo Branch $BRANCH_NAME is supported. Continuing.
                                version=`mvn help:evaluate -Dexpression=project.version -q -DforceStdout`
                                echo version=$version
                                case "$version" in
                                *"SNAPSHOT"*) echo echo version is OK ;;
                                *       ) echo echo "You cannot build releases on Jenkins, only snapshots!"&&exit 1 ;;
                                esac
                                mvn clean deploy
        		        ;;
    		        
      		        *)
        		        echo Branch $BRANCH_NAME is not supported. A failure happened. Exiting.
                        exit 1
        		        ;;
    		        esac
    		        
    		        echo "Build of $JOB_NAME was successful"
    		        '''
            }
        }
        
        stage('Deploy')
        {
            steps {
                echo "*** Deploying ${env.JOB_NAME} ***"
              
    		    sh '''
    		        #!/bin/bash
    		        
    		        echo "Nothing to do"
    		        exit

    		        case $BRANCH_NAME in

    		          master | deploy_prod)
                        echo Branch $BRANCH_NAME is supported. Continuing.
                        TOMCAT_HOME=$TOMCAT10_HOME
                        systemdService=tomcat10
        		        ;;
    		        
      		          develop | jenkins | deploy_test)
        		        echo Branch $BRANCH_NAME is supported. Continuing.
                        TOMCAT_HOME=$TOMCAT10_TEST_HOME
                        systemdService=tomcat10test
        		        ;;
    		        
      		        *)
        		        echo Branch $BRANCH_NAME is not supported. A failure happened. Exiting.
                        exit 1
        		        ;;
    		        esac

    		       '''
	          
            }
        }
    }
    post {
        always {
            script {
                env.color = "${currentBuild.currentResult == 'SUCCESS' ? 'green' : 'red'}"
           }
            
            echo 'Sending e-mail.'
            sh "printenv | sort"
            emailext body: "<b style=\"color:$COLOR\">${currentBuild.currentResult}</b> - ${env.JOB_NAME} (#${env.BUILD_NUMBER})<br> <ul style=\"margin-top:2px;padding-top:2px;padding-left:30px;\"><li>More info at: <a href=\"${env.BUILD_URL}\">${env.BUILD_URL}</a></li></ul>",
                recipientProviders: [[$class: 'DevelopersRecipientProvider'], [$class: 'RequesterRecipientProvider']],
                subject: "Jenkins Build - ${currentBuild.currentResult} - $JOB_NAME (#$BUILD_NUMBER)"
            
        }
    }
}
