(:*******************************************************:)
(:Test: op-subtract-dayTimeDuration-from-date-12         :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The string value of "subtract-dayTimeDuration-from-date" operator used :)
(:with a boolean expression and the "fn:true" function.   :)
(:*******************************************************:)
 
fn:string((xs:date("1980-03-02Z") - xs:dayTimeDuration("P05DT08H11M"))) and (fn:true())