(:*******************************************************:)
(:Test: op-yearMonthDuration-greater-than-14             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "yearMonthDuration-greater-than" function used :)
(:together with "fn:false"/or expression (ge operator).  :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P10Y05M") ge xs:yearMonthDuration("P20Y10M")) or (fn:false())