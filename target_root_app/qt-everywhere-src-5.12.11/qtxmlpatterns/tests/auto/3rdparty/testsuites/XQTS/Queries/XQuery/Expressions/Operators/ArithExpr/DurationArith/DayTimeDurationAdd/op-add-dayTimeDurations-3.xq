(:*******************************************************:)
(:Test: op-add-dayTimeDurations-3                        :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "add-dayTimeDurations" function as :)
(:part of a boolean expression (or operator) and the "fn:boolean" function. :)
(: Uses the "fn:string" function to account for new EBV rules. :)
(:*******************************************************:)
 
fn:string(xs:dayTimeDuration("P20DT20H10M") + xs:dayTimeDuration("P19DT13H10M")) or fn:false()