(:*******************************************************:)
(:Test: op-subtract-dates-yielding-DTD-19                :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dates-yielding-DTD" operator as :)
(:part of a multiplication expression                     :)
(:*******************************************************:)
 
(xs:date("1999-10-23Z") - xs:date("1998-09-09Z")) * xs:decimal(2.0)