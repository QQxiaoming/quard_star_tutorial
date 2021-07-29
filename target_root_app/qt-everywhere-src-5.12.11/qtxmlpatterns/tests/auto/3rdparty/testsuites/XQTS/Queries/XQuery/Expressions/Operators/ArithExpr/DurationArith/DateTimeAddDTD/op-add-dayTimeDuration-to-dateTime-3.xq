(:*******************************************************:)
(:Test: op-add-dayTimeDuration-to-dateTime-3             :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The string value of the "add-dayTimeDuration-to-dateTime" :)
(:operator as part of a boolean expression (or operator) and the "fn:boolean" :)
(:function. :)
(:*******************************************************:)
 
fn:string((xs:dateTime("1999-10-23T03:12:23Z") + xs:dayTimeDuration("P19DT13H10M"))) or fn:false()