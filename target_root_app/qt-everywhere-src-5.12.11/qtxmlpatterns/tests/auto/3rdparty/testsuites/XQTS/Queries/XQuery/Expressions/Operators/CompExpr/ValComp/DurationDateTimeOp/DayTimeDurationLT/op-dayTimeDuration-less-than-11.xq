(:*******************************************************:)
(:Test: op-dayTimeDuration-less-than-11                  :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dayTimeDuration-less-than" function used  :)
(:together with "fn:true"/or expression (lt operator).   :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P10DT02H") lt xs:dayTimeDuration("P01DT10H")) or (fn:true())