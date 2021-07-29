(:*******************************************************:)
(:Test: op-subtract-dayTimeDuration-from-time-12         :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The string value of "subtract-dayTimeDuration-from-time" operator used :)
(:with a boolean expression and the "fn:true" function.   :)
(:*******************************************************:)
 
fn:string((xs:time("02:02:02Z") - xs:dayTimeDuration("P05DT08H11M"))) and (fn:true())