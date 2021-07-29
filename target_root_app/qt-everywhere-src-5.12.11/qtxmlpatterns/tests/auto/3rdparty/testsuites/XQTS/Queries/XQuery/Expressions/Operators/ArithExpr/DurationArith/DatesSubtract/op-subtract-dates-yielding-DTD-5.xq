(:*******************************************************:)
(:Test: op-subtract-dates-yielding-DTD-5                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 3, 2005                                     :)
(:Purpose: Evaluates The "subtract-dates-yielding-DTD" operator that  :)
(:is used as an argument to the fn:boolean function.     :)
(:*******************************************************:)
 
fn:boolean(fn:string(xs:date("1962-03-12Z") - xs:date("1962-03-12Z")))