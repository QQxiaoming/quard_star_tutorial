(:*******************************************************:)
(:Test: op-add-yearMonthDuration-to-date-9               :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The string value of "add-yearMonthDuration-to-date" operator used  :)
(:together with an "and" expression.                      :)
(:*******************************************************:)
 
fn:string((xs:date("1993-12-09Z") + xs:yearMonthDuration("P03Y03M"))) and fn:string((xs:date("1993-12-09Z") + xs:yearMonthDuration("P03Y03M")))