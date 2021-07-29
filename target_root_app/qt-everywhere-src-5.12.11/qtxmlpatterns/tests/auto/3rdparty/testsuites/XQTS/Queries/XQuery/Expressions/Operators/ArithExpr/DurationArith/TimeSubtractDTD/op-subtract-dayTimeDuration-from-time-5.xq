(:*******************************************************:)
(:Test: op-subtract-dayTimeDuration-from-time-5          :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The string value of "subtract-dayTimeDuration-from-time" operator that  :)
(:is used as an argument to the fn:boolean function.     :)
(:*******************************************************:)
 
fn:boolean(fn:string(xs:time("23:55:55Z") - xs:dayTimeDuration("P03DT08H06M")))