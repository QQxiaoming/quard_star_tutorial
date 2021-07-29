(:*******************************************************:)
(:Test: op-subtract-dayTimeDuration-from-date-6          :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "subtract-dayTimeDuration-from-date" operator that :)
(:is used as an argument to the fn:number function.      :)
(:*******************************************************:)
 
fn:number(xs:date("1988-01-28Z") - xs:dayTimeDuration("P10DT08H01M"))