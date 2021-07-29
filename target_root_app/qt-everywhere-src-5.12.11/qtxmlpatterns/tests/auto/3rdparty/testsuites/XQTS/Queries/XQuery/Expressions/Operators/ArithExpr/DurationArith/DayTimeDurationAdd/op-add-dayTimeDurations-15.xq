(:*******************************************************:)
(:Test: op-add-dayTimeDurations-15                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "add-dayTimeDurations" function used  :)
(:together with the numeric-equal operator "le".         :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P10DT10H01M") + xs:dayTimeDuration("P17DT10H02M")) le xs:dayTimeDuration("P17DT10H02M")