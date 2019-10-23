#include "AbsolutelyNotRobot.h"
#define PIN_CODEUSE_GAUCHE_A 29
#define PIN_CODEUSE_GAUCHE_B 28
#define PIN_CODEUSE_DROITE_A 34
#define PIN_CODEUSE_DROITE_B 35

#define PIN_MOTEUR_GAUCHE_PWR 22
#define PIN_MOTEUR_GAUCHE_SENS 36
#define PIN_MOTEUR_GAUCHE_BRAKE 37

#define PIN_MOTEUR_DROITE_PWR 23
#define PIN_MOTEUR_DROITE_SENS 38
#define PIN_MOTEUR_DROITE_BRAKE 39

#define ELOIGNEMENT_CODEUSES 0.29430415
#define DIAMETRE_ROUE_CODEUSE_DROITE 0.05325315
#define DIAMETRE_ROUE_CODEUSE_GAUCHE 0.053570956
#define TICKS_PER_ROUND 16384

void AbsolutelyNotRobot::update(float dt)
{
    communication.update();
    if (useSimulator)
    {
        simu.updateCinetique(dt);
    }
    else
    {
        odometrie.updateCinetique(dt);
    }
    ghost.ActuatePosition(dt);
    target = ghost.Get_Controller_Cinetique();
    asservissement.compute(dt);
    motorLeft.setOrder(outTranslation - outRotation);
    motorRight.setOrder(outTranslation + outRotation);
    motorLeft.actuate();
    motorRight.actuate();
    sequence.update();
    if (communication.inWaiting() > 0)
        communication.pullOldestMessage(); //Tout le monde a eu l'occasion de le peek, on le vire.
}


AbsolutelyNotRobot::AbsolutelyNotRobot(float x, float y, float theta, bool useSimulator)
{
    this->useSimulator = useSimulator;
    cin = Cinetique(x, y, theta);
    if (not useSimulator)
    {
        odometrie = Odometrie(TICKS_PER_ROUND, &cin, ELOIGNEMENT_CODEUSES, PIN_CODEUSE_GAUCHE_A, PIN_CODEUSE_GAUCHE_B, DIAMETRE_ROUE_CODEUSE_GAUCHE, PIN_CODEUSE_DROITE_A, PIN_CODEUSE_DROITE_B, DIAMETRE_ROUE_CODEUSE_DROITE);
        Serial.println("REAL MODE");
    }
    motorLeft = Motor(PIN_MOTEUR_GAUCHE_PWR, PIN_MOTEUR_GAUCHE_SENS, 10);
    pinMode(PIN_MOTEUR_DROITE_BRAKE, OUTPUT);
    digitalWrite(PIN_MOTEUR_DROITE_BRAKE, LOW); //Adaptation ancien driver
    motorRight = Motor(PIN_MOTEUR_DROITE_PWR, PIN_MOTEUR_DROITE_SENS, 10);
    pinMode(PIN_MOTEUR_GAUCHE_BRAKE, OUTPUT);
    digitalWrite(PIN_MOTEUR_GAUCHE_BRAKE, LOW); //Adaptation ancien driver
    ghost = Ghost(cin);
    asservissement = Asservissement(&outTranslation, &outRotation, &cin, &target, 30);
    communication = Communication();
    sequence = Sequence();
    Action::setPointers(&cin, &ghost, &sequence, &communication, &asservissement);
    //sequence.add(new Wait_Message_Action(Tirette,-1));
    sequence.add(new StraightTo_Action(-1,1,1,standard));
    sequence.add(new Spin_Action(-1, 3.14, standard));
    sequence.add(new Goto_Action(-1, 0.0, 0.0, -1.57, 0.2, standard));
    sequence.add(new End_Action());
    if (useSimulator)
    {
        simu = Simulator(0.30, 10.0, 6.5, 1.5, &cin, &motorLeft.order, &motorRight.order);
        Serial.println("SIMULATOR MODE");
    }
    sequence.reStart();
    ghost.Lock(false);
}

void AbsolutelyNotRobot::debug(bool showRobotCin, bool showGhostCin, bool showCurrentActionState, bool showCurrentIndex, bool showSequence)
{
    if (showRobotCin)
    {
        Serial.print("Robot: ");
        cin.print();
    }
    if (showGhostCin)
    {
        Serial.print("Ghost: ");
        ghost.Get_Controller_Cinetique().print();
    }
    if (showCurrentActionState)
        sequence.debug(showCurrentActionState,showCurrentIndex,showSequence);
    Serial.println();
}