(:*******************************************************:)
(:Test: op-yearMonthDuration-greater-than-13             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "yearMonthDuration-greater-than" function used  :)
(:together with "fn:false"/or expression (gt operator).  :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P30Y10M") gt xs:yearMonthDuration("P01Y02M")) or (fn:false())