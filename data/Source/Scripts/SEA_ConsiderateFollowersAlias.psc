Scriptname SEA_ConsiderateFollowersAlias Extends ReferenceAlias 

Event OnPlayerLoadGame()

    (Self.GetOwningQuest() As SEA_ConsiderateFollowersQuest).Maintenance()
EndEvent