(:*******************************************************:)
(:Test: op-yearMonthDuration-less-than-11                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "yearMonthDuration-less-than" function used  :)
(:together with "fn:true"/or expression (lt operator).   :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P10Y02M") lt xs:yearMonthDuration("P01Y10M")) or (fn:true())