(:*******************************************************:)
(:Test: op-subtract-dates-yielding-DTD-6                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dates-yielding-DTD" operator that :)
(:is used as an argument to the fn:number function.      :)
(:*******************************************************:)
 
fn:number(xs:date("1988-01-28Z") - xs:date("2001-03-02"))