(:*******************************************************:)
(:Test: op-add-dayTimeDuration-to-date-5                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The string value  "add-dayTimeDuration-to-date" operator that  :)
(:is used as an argument to the fn:boolean function.     :)
(:*******************************************************:)
 
fn:boolean(fn:string(xs:date("1962-03-12Z") + xs:dayTimeDuration("P03DT08H06M")))