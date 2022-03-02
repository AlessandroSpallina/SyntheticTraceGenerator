package syntetictracegenerator.simulations.Villarosa_gnb_only_test_non_mesh;

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
        visualizer: IntegratedVisualizer {
            @display("p=50,328");
        }
        roadsCanvasVisualizer: RoadsCanvasVisualizer {
            @display("p=50,397");
        }
        server: StandardHost {
            @display("p=3128.125,395.39502;is=n;i=device/server");
        }
        router: Router {
            @display("p=2667.665,390.39;i=device/smallrouter");
        }
        upf: Upf {
            @display("p=2137.135,255.255;is=l");
        }
        gNodeB1: gNodeB {
            @display("p=1116.115,910.91003;is=vl");
        }
        gNodeB2: gNodeB {
            @display("p=1801.8,910.91003;is=vl");
        }
        gNodeB3: gNodeB {
            @display("p=2537.5352,910.91003;is=vl");
        }


    connections allowunconnected:
        server.pppg++ <--> Eth10G <--> router.pppg++;
        router.pppg++ <--> Eth10G <--> upf.filterGate;
        upf.pppg++ <--> Eth10G <--> gNodeB1.ppp;
        upf.pppg++ <--> Eth10G <--> gNodeB2.ppp;
        upf.pppg++ <--> Eth10G <--> gNodeB3.ppp;

        //# X2 connections
        gNodeB1.x2++ <--> Eth10G <--> gNodeB2.x2++;
        gNodeB2.x2++ <--> Eth10G <--> gNodeB3.x2++;
        //gNodeB1.x2++ <--> Eth10G <--> gNodeB3.x2++;


}

