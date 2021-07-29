(:*******************************************************:)
(:Test: op-add-dayTimeDurations-16                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "add-dayTimeDurations" function used  :)
(:together with the numeric-equal operator "ge".         :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P13DT09H09M") + xs:dayTimeDuration("P18DT02H02M")) ge  xs:dayTimeDuration("P18DT02H02M")