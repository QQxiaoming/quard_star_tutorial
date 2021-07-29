(:*******************************************************:)
(:Test: op-dayTimeDuration-less-than-12                  :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dayTimeDuration-less-than" function used  :)
(:together with "fn:true"/or expression (le operator).   :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P10DT01H") le xs:dayTimeDuration("P09DT05H")) or (fn:true())