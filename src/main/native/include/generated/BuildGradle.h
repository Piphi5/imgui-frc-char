#pragma once

constexpr char kBuildGradleContents[] = R"(plugins {
    id "java"
    id "edu.wpi.first.GradleRIO" version "2021.1.1-beta-1"
}

sourceCompatibility = JavaVersion.VERSION_11
targetCompatibility = JavaVersion.VERSION_11

def ROBOT_MAIN_CLASS = "frc.robot.Main"

// Define my targets (RoboRIO) and artifacts (deployable files)
// This is added by GradleRIO's backing project EmbeddedTools.
deploy {
    targets {
        roboRIO("roborio") {
            // Team number is loaded either from the .wpilib/wpilib_preferences.json
            // or from command line. If not found an exception will be thrown.
            // You can use getTeamOrDefault(team) instead of getTeamNumber if you
            // want to store a team number in this file.
            team = frc.getTeamNumber()
        }
    }
    artifacts {
        frcJavaArtifact('frcJava') {
            targets << "roborio"
            // Debug can be overridden by command line, for use with VSCode
            debug = frc.getDebugOrDefault(false)
        }
        // Built in artifact to deploy arbitrary files to the roboRIO.
        fileTreeArtifact('frcStaticFileDeploy') {
            // The directory below is the local directory to deploy
            files = fileTree(dir: 'src/main/deploy')
            // Deploy to RoboRIO target, into /home/lvuser/deploy
            targets << "roborio"
            directory = '/home/lvuser/deploy'
        }
    }
}

// Set this to true to enable desktop support.
def includeDesktopSupport = true

// Defining my dependencies. In this case, WPILib (+ friends), and vendor libraries.
// Also defines JUnit 4.
dependencies {
    implementation wpi.deps.wpilib()
    nativeZip wpi.deps.wpilibJni(wpi.platforms.roborio)
    nativeDesktopZip wpi.deps.wpilibJni(wpi.platforms.desktop)


    implementation wpi.deps.vendor.java()
    nativeZip wpi.deps.vendor.jni(wpi.platforms.roborio)
    nativeDesktopZip wpi.deps.vendor.jni(wpi.platforms.desktop)

    testImplementation 'junit:junit:4.12'

    // Enable simulation gui support. Must check the box in vscode to enable support
    // upon debugging
    simulation wpi.deps.sim.gui(wpi.platforms.desktop, false)
    simulation wpi.deps.sim.driverstation(wpi.platforms.desktop, false)

    // Websocket extensions require additional configuration.
    // simulation wpi.deps.sim.ws_server(wpi.platforms.desktop, false)
    // simulation wpi.deps.sim.ws_client(wpi.platforms.desktop, false)
}

// Setting up my Jar File. In this case, adding all libraries into the main jar ('fat jar')
// in order to make them all available at runtime. Also adding the manifest so WPILib
// knows where to look for our Robot Class.
jar {
    from { configurations.runtimeClasspath.collect { it.isDirectory() ? it : zipTree(it) } }
    manifest edu.wpi.first.gradlerio.GradleRIOPlugin.javaManifest(ROBOT_MAIN_CLASS)
}
)";