(:*******************************************************:)
(:Test: op-dayTimeDuration-greater-than-12               :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dayTimeDuration-greater-than" function used :)
(:together with "fn:true"/or expression (ge operator).   :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P10DT01H") ge xs:dayTimeDuration("P09DT05H")) or (fn:true())