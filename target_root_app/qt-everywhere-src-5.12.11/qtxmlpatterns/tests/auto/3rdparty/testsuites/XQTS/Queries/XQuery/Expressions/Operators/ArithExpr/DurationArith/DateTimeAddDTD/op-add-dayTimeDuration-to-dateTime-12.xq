(:*******************************************************:)
(:Test: op-add-dayTimeDuration-to-dateTime-12            :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The string value of the "add-dayTimeDuration-to-dateTime" :)
(:operator used with a boolean expression and the "fn:true" function. :)
(:*******************************************************:)
 
fn:string((xs:dateTime("1980-03-02T16:12:10Z") + xs:dayTimeDuration("P05DT08H11M"))) and (fn:true())