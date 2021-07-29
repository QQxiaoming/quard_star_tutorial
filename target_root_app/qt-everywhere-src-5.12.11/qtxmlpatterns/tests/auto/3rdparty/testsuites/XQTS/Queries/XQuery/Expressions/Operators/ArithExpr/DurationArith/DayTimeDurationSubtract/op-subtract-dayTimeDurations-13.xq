(:*******************************************************:)
(:Test: op-subtract-dayTimeDurations-13                  :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "subtract-dayTimeDurations" function used  :)
(:together with the numeric-equal-operator "eq".         :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P23DT11H11M") - xs:dayTimeDuration("P23DT11H11M")) eq xs:dayTimeDuration("P23DT11H11M")