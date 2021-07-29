(:*******************************************************:)
(:Test: op-add-dayTimeDuration-to-time-6                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "add-dayTimeDuration-to-time" operator that :)
(:is used as an argument to the fn:number function.      :)
(:*******************************************************:)
 
fn:number(xs:time("01:01:01Z") + xs:dayTimeDuration("P10DT08H01M"))