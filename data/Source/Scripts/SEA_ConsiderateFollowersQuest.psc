Scriptname SEA_ConsiderateFollowersQuest extends Quest  

Import SEA_ConsiderateFollowers

Function Maintenance()

    RegisterForAllEvents(Self)
EndFunction

Event OnInit()

    Maintenance()
EndEvent

Event OnFollowerShouldComment(Actor akFollower, Topic akTopic)

    akFollower.Say(akTopic)
    debug.notification("Spoke?")
EndEvent