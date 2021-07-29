(:*******************************************************:)
(:Test: op-yearMonthDuration-less-than-12                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "yearMonthDuration-less-than" function used  :)
(:together with "fn:true"/or expression (le operator).   :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P10Y01M") le xs:yearMonthDuration("P09Y05M")) or (fn:true())