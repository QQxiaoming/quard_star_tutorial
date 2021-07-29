(: Name: ForExprType031 :)
(: Description: FLWOR expressions with type declaration. Value subtype promotion: Time based types -> anyAtomicType :)

for $test as xs:anyAtomicType in ( xs:duration("P0Y1347M0D"), xs:dateTime("1999-05-31T13:20:00-05:00"), xs:time("13:00:00-05:00"), xs:date("1999-05-10Z"), xs:gYearMonth("2002-03"), xs:gYear("1999"), xs:gMonthDay("--05-10"), xs:gDay("---10"), xs:gMonth("--05") )
return $test