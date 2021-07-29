(:*******************************************************:)
(:Test: op-subtract-yearMonthDurations-7                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 28, 2005                                    :)
(:Purpose: Evaluates The "subtract-yearMonthDurations" function used  :)
(:as an argument to the "fn:string" function).           :)
(:*******************************************************:)
 
fn:string(xs:yearMonthDuration("P03Y08M") - xs:yearMonthDuration("P01Y02M"))