(:*******************************************************:)
(:Test: op-dayTimeDuration-greater-than-14               :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dayTimeDuration-greater-than" function used :)
(:together with "fn:false"/or expression (ge operator).  :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P10DT05H") ge xs:dayTimeDuration("P20DT10H")) or (fn:false())