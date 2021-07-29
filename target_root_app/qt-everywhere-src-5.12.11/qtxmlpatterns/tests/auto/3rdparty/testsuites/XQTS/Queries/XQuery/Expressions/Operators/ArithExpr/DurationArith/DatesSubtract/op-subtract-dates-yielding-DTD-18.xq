(:*******************************************************:)
(:Test: op-subtract-dates-yielding-DTD-18                :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dates-yielding-DTD" operator as :)
(:part of a boolean expression (or operator) and the "fn:false" function. :)
(:*******************************************************:)
 
fn:string((xs:date("1999-10-23Z") - xs:date("1998-09-09Z"))) or fn:false()