(:*******************************************************:)
(:Test: op-yearMonthDuration-greater-than-11             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "yearMonthDuration-greater-than" function used  :)
(:together with "fn:true"/or expression (gt operator).   :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P10Y02M") gt xs:yearMonthDuration("P01Y10M")) or (fn:true())