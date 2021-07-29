(:*******************************************************:)
(:Test: op-add-dayTimeDuration-to-dateTime-6             :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "add-dayTimeDuration-to-dateTime" operator that :)
(:is used as an argument to the fn:number function.      :)
(:*******************************************************:)
 
fn:number(xs:dateTime("1988-01-28T12:34:12Z") + xs:dayTimeDuration("P10DT08H01M"))