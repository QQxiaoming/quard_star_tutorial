(:*******************************************************:)
(:Test: op-dayTimeDuration-greater-than-11               :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dayTimeDuration-greater-than" function used  :)
(:together with "fn:true"/or expression (gt operator).   :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P10DT02H") gt xs:dayTimeDuration("P01DT10H")) or (fn:true())