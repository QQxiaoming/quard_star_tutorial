(:*******************************************************:)
(:Test: op-subtract-dayTimeDuration-from-time-4          :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The string value of "subtract-dayTimeDuration-from-time" operator that  :)
(:return true and used together with fn:not.             :)
(:*******************************************************:)
 
fn:not(fn:string(xs:time("20:50:50Z") - xs:dayTimeDuration("P02DT07H01M")))