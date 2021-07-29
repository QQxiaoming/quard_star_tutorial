(:*******************************************************:)
(:Test: op-yearMonthDuration-less-than-14                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "yearMonthDuration-less-than" function used  :)
(:together with "fn:false"/or expression (le operator).  :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P10Y05M") le xs:yearMonthDuration("P20Y10M")) or (fn:false())