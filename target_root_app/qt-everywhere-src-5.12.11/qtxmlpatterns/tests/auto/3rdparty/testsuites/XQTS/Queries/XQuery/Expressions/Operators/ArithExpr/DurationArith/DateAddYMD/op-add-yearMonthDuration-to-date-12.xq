(:*******************************************************:)
(:Test: op-add-yearMonthDuration-to-date-12              :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The string value "add-yearMonthDuration-to-date" operator used :)
(:with a boolean expression and the "fn:true" function.   :)
(:*******************************************************:)
 
fn:string((xs:date("1980-03-02Z") + xs:yearMonthDuration("P05Y05M"))) and (fn:true())