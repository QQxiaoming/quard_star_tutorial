(:*******************************************************:)
(:Test: op-dayTimeDuration-less-than-14                  :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dayTimeDuration-less-than" function used  :)
(:together with "fn:false"/or expression (le operator).  :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P10DT05H") le xs:dayTimeDuration("P20DT10H")) or (fn:false())