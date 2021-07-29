(:*******************************************************:)
(:Test: op-yearMonthDuration-less-than-13                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "yearMonthDuration-less-than" function used  :)
(:together with "fn:false"/or expression (lt operator).  :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P30Y10M") lt xs:yearMonthDuration("P01Y02M")) or (fn:false())