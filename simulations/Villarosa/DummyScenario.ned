// 
//                  Simu5G
//
// Authors: Giovanni Nardini, Giovanni Stea, Antonio Virdis (University of Pisa)
// 
// This file is part of a software released under the license included in file
// "license.pdf". Please read LICENSE and README files before using it.
// The above files and the present reference are part of the software itself, 
// and cannot be removed from it.
//
package syntetictracegenerator.simulations.Villarosa;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.inet.AdhocHost;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.ethernet.Eth10G;

import simu5g.world.radio.LteChannelControl;
import simu5g.common.carrierAggregation.CarrierAggregation;
import simu5g.nodes.Upf;
import simu5g.common.binder.Binder;
import simu5g.nodes.NR.gNodeB;
import simu5g.nodes.cars.Car;

import org.car2x.veins.subprojects.veins_inet.VeinsInetManager;

import inet.visualizer*.common.IntegratedVisualizer;
import org.car2x.veins.visualizer.roads.RoadsCanvasVisualizer;

import simu5g.nodes.mec.MECHost;
import simu5g.nodes.mec.MECOrchestrator.MecOrchestrator;
import simu5g.nodes.mec.UALCMP.UALCMP;

network DummyScenario
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)

        @display("bgb=4000,2000");
    submodules:

        routingRecorder: RoutingTableRecorder {
            @display("p=50,75;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=50,125");
            config = xmldoc("demo.xml");
        }

        //# Veins manager module
        veinsManager: VeinsInetManager {
            @display("p=50,227;is=s");
        }

        //# LTE modules
        channelControl: LteChannelControl {
            @display("p=50,25;is=s");
        }
        binder: Binder {
            @display("p=50,175;is=s");
        }
        carrierAggregation: CarrierAggregation {
            @display("p=50.993748,258.7;is=s");
        }
        //visualizer: IntegratedVisualizer {
        //    @display("p=50,328");
        //}
        roadsCanvasVisualizer: RoadsCanvasVisualizer {
            @display("p=50,397");
        }
        upf: Upf {
            @display("p=1945.004,671.1835;is=l");
        }
        gNodeB1: gNodeB {
            @display("p=1910.7308,225.63188;is=vl");
        }
        gNodeB2: gNodeB {
            @display("p=1373.784,1165.2887;is=vl");
        }
        gNodeB3: gNodeB {
            @display("p=2656.1729,956.79346;is=vl");
        }
        mecHost: MECHost {
            @display("p=2047.8236,1256.684;is=l");
        }

        mecOrchestrator: MecOrchestrator {
            @display("p=1710.8038,945.369;is=l");
        }

        ualcmp: UALCMP {
            @display("p=1947.8601,851.11774;i=device/smallrouter");
        }

    connections allowunconnected:
        //server.pppg++ <--> Eth10G <--> router.pppg++;
        // router.pppg++ <--> Eth10G <--> upf.filterGate;
        upf.pppg++ <--> Eth10G <--> gNodeB1.ppp;
        upf.pppg++ <--> Eth10G <--> gNodeB2.ppp;
        upf.pppg++ <--> Eth10G <--> gNodeB3.ppp;

        ualcmp.ppp++ <--> Eth10G <--> upf.filterGate;

        ualcmp.toMecOrchestrator --> mecOrchestrator.fromUALCMP;
        ualcmp.fromMecOrchestrator <-- mecOrchestrator.toUALCMP;

        //# X2 connections
        gNodeB1.x2++ <--> Eth10G <--> gNodeB2.x2++;
        gNodeB2.x2++ <--> Eth10G <--> gNodeB3.x2++;
        gNodeB1.x2++ <--> Eth10G <--> gNodeB3.x2++;

        mecHost.pppENB++ <--> Eth10G <--> gNodeB1.pppMEHost;
        mecHost.pppENB++ <--> Eth10G <--> gNodeB2.pppMEHost;
        mecHost.pppENB++ <--> Eth10G <--> gNodeB3.pppMEHost;


}

