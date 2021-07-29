(:*******************************************************:)
(:Test: op-dayTimeDuration-less-than-13                  :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dayTimeDuration-less-than" function used  :)
(:together with "fn:false"/or expression (lt operator).  :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P30DT10H") lt xs:dayTimeDuration("P01DT02H")) or (fn:false())