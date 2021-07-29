(:*******************************************************:)
(:Test: op-yearMonthDuration-greater-than-12             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "yearMonthDuration-greater-than" function used  :)
(:together with "fn:true"/or expression (ge operator).   :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P10Y01M") ge xs:yearMonthDuration("P09Y05M")) or (fn:true())