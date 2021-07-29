(:*******************************************************:)
(:Test: op-add-dayTimeDuration-to-time-3                 :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "add-dayTimeDuration-to-time" function as :)
(:part of a boolean expression (or operator) and the "fn:boolean" function. :)
(: Uses the "fn:string" function to account for new EBV rules. :)
(:*******************************************************:)
 
fn:string((xs:time("12:12:01Z") + xs:dayTimeDuration("P19DT13H10M"))) or fn:false()