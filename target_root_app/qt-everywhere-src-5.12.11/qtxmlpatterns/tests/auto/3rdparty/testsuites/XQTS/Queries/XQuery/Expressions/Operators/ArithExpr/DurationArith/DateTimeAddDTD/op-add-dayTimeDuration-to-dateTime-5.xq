(:*******************************************************:)
(:Test: op-add-dayTimeDuration-to-dateTime-5             :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The string value of the "add-dayTimeDuration-to-dateTime" :)
(:operator that is used as an argument to the fn:boolean function. :)
(:*******************************************************:)
 
fn:boolean(fn:string(xs:dateTime("1962-03-12T12:34:09Z") + xs:dayTimeDuration("P03DT08H06M")))