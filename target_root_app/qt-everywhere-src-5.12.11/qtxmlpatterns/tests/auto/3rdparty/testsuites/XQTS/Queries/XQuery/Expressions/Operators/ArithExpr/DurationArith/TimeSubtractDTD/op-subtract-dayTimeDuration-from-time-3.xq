(:*******************************************************:)
(:Test: op-subtract-dayTimeDuration-from-time-3          :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The string value "subtract-dayTimeDuration-from-time" operator as :)
(:part of a boolean expression (or operator) and the "fn:boolean" function. :)
(:*******************************************************:)
 
fn:string((xs:time("12:12:01Z") - xs:dayTimeDuration("P19DT13H10M"))) or fn:false()