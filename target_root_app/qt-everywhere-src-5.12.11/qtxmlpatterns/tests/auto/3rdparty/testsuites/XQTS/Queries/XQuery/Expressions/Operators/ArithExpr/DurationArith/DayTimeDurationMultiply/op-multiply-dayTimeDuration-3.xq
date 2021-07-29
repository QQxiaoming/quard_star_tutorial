(:*******************************************************:)
(:Test: op-multiply-dayTimeDuration-3                    :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "multiply-dayTimeDuration" function as :)
(:part of a boolean expression (or operator) and the "fn:boolean" function. :)
(:*******************************************************:)
 
fn:string((xs:dayTimeDuration("P20DT20H10M") * 2.0)) or fn:false()