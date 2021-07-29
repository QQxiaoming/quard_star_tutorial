(:*******************************************************:)
(:Test: op-subtract-yearMonthDuration-from-date-12       :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The string value of "subtract-yearMonthDuration-from-date" operator used :)
(:with a boolean expression and the "fn:true" function.   :)
(:*******************************************************:)
 
fn:string((xs:date("1980-03-02Z") - xs:yearMonthDuration("P05Y05M"))) and (fn:true())