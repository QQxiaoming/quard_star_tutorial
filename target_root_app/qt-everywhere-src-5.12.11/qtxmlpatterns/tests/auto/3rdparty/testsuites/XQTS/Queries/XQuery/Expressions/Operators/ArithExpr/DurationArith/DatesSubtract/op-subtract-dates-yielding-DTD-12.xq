(:*******************************************************:)
(:Test: op-subtract-dates-yielding-DTD-12                :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dates-yielding-DTD" operator used :)
(:with a boolean expression and the "fn:true" function.   :)
(:*******************************************************:)
 
fn:string((xs:date("1980-03-02Z") - xs:date("2001-09-11Z"))) and (fn:true())