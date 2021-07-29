(:*******************************************************:)
(:Test: op-subtract-dateTimes-yielding-DTD-19            :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dates-yielding-DTD" operator as :)
(:part of a multiplication expression                     :)
(:*******************************************************:)
 
(xs:dateTime("1999-10-23T01:01:01Z") - xs:dateTime("1998-09-09T02:02:02Z")) * xs:decimal(2.0)