(:*******************************************************:)
(:Test: op-subtract-dayTimeDurations-5                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29 2005                                     :)
(:Purpose: Evaluates The "subtract-dayTimeDurations" function that  :)
(:is used as an argument to the fn:boolean function.     :)
(:*******************************************************:)
 
fn:boolean(fn:string(xs:dayTimeDuration("P05DT09H08M") - xs:dayTimeDuration("P03DT08H06M")))