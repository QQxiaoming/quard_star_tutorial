(:*******************************************************:)
(:Test: op-subtract-dayTimeDurations-14                  :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "subtract-dayTimeDurations" function used  :)
(:together with the numeric-equal operator "ne".         :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P21DT08H12M") - xs:dayTimeDuration("P08DT08H05M")) ne xs:dayTimeDuration("P08DT08H05M")