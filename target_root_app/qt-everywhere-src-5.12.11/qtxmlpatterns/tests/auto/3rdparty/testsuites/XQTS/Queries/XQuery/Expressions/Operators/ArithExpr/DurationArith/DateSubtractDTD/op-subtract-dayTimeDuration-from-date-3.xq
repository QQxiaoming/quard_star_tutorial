(:*******************************************************:)
(:Test: op-subtract-dayTimeDuration-from-date-3          :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The string value of "subtract-dayTimeDuration-from-date" operator as :)
(:part of a boolean expression (or operator) and the "fn:boolean" function. :)
(:*******************************************************:)
 
fn:string((xs:date("1999-10-23Z") - xs:dayTimeDuration("P19DT13H10M"))) or fn:false()