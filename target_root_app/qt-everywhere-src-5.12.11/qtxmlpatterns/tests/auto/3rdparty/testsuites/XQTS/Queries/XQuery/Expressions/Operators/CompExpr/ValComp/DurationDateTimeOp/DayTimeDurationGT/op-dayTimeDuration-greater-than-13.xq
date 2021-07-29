(:*******************************************************:)
(:Test: op-dayTimeDuration-greater-than-13               :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dayTimeDuration-greater-than" function used :)
(:together with "fn:false"/or expression (gt operator).  :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P30DT10H") gt xs:dayTimeDuration("P01DT02H")) or (fn:false())