(:*******************************************************:)
(:Test: op-subtract-dayTimeDuration-from-time-6          :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "subtract-dayTimeDuration-from-time" operator that :)
(:is used as an argument to the fn:number function.      :)
(:*******************************************************:)
 
fn:number(xs:time("10:11:45Z") - xs:dayTimeDuration("P10DT08H01M"))