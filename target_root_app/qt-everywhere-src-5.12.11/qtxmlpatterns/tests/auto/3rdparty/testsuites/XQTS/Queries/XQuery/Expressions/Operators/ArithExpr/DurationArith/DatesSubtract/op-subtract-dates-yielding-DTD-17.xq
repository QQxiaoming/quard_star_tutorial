(:*******************************************************:)
(:Test: op-subtract-dates-yielding-DTD-17                :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dates-yielding-DTD" operator :)
(:used as part of a boolean expression (and operator) and the "fn:false" function. :)
(:*******************************************************:)

fn:string(xs:date("2000-12-12Z") - xs:date("2000-11-11Z")) and fn:false()