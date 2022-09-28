#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream.h>
#include <SDtMail/Time.hh>
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/String.hh>
#include "Str.hh"
#include <SDtMail/Vector.hh>
#include <SDtMail/PtrVector.hh>
#include <SDtMail/SortVector.hh>
#include <SDtMail/PtrSortVector.hh>
#include <SDtMail/LinkedList.hh>
#include <SDtMail/DoubleLinkedList.hh>
#include <DataStructs/RecursiveMutexEntry.hh>

SdmErrorCode RunDataTypeTest ();
static void exercise_string ();
static void exercise_string_append ();
static void exercise_vector ();
static void exercise_ptr_vector ();
static void exercise_sort_vector ();
static void exercise_ptr_sort_vector ();
static void exercise_dlist ();
static void exercise_time ();
static void exercise_recursive_mutex ();
static int status = 1;

SdmErrorCode RunDataTypeTest ()
{
	SdmError err;

	cout << "\n-------------------------------------------\n";
	exercise_string_append();
	cout << "\n-------------------------------------------\n";
	exercise_string();
	cout << "\n-------------------------------------------\n";
	exercise_vector();
	cout << "\n-------------------------------------------\n";
	exercise_ptr_vector();
	cout << "\n-------------------------------------------\n";
	exercise_sort_vector();
	cout << "\n-------------------------------------------\n";
	exercise_ptr_sort_vector();
	cout << "\n-------------------------------------------\n";
	exercise_dlist();
	cout << "\n-------------------------------------------\n";
	exercise_time();
	cout << "\n-------------------------------------------\n";

#if defined(USE_SOLARIS_THREADS)
        // recursive mutex only works for multi-thread mid-end.
	exercise_recursive_mutex();
#endif
	cout << "\n-------------------------------------------\n";

	if (!status)
		err = Sdm_EC_Fail;
	return err;
}

static void exercise_string_append ()
{
  char str[2001];
  char str2[48];
  for (int i=0; i<2000; i++) {
    str[i] = '0';
  }
  str[i] = '\0';

  SdmString s1(str);

  for (i=0; i<47; i++) {
    str2[i] = '1';
  }
  str2[i] = '\0';

  s1 += str2;

  
/*
  SdmMessageNumberL msgnums;

  for (int i=0; i<= 1659; i++) {
     msgnums.AddElementToList(i);
  }

  SdmString msgno_predicate;
  SdmVectorIterator<SdmMessageNumber> iter(&msgnums);
  char buffer[256];
  SdmBoolean first = Sdm_True;
  SdmMessageNumber *msgnum_ptr;


  while ((msgnum_ptr = iter.NextListElement()) != NULL) {
    if (first) {
      sprintf(buffer, "%ld", *msgnum_ptr);
      first = Sdm_False;
    } else {
      sprintf (buffer, ",%ld", *msgnum_ptr);
    }
    msgno_predicate += buffer;
  }
  fprintf(stderr, "%s\n", (const char*) msgno_predicate);
*/
}

static void exercise_string ()
{
	SdmString str ("hello there");
	SdmString substr;
	SdmString a("abcd"), b("efgh");
	SdmString c ("hmm");
	SdmString d ('z', 7);
	SdmString e (c);
	SdmString f ("yo");
	SdmString k ('.', 30);
	SdmString g, h, l;
	SdmString null_str;
	SdmString x ("good bye");
	SdmString y;
	int pos, num = 1;

	cout << "SdmString Test" << endl;
	cout << "--------------" << endl << endl;

	if (b.Length() == 4)
	{
		cout << "PASS\tLength()" << endl;
	}
	else
	{
		cout << "FAIL\tLength()" << endl;
		status = 0;
	}

	f = a + b;
	g = null_str + d;
	h = d + null_str;
	if ( (strcmp (f, "abcdefgh") == 0)
	  && (strcmp (g, "zzzzzzz") == 0)
	  && (strcmp (h, "zzzzzzz") == 0))
	{
		cout << "PASS\tconcatenation operator" << endl;
	}
	else
	{
		cout << "FAIL\tconcatenation operator" << endl;
		status = 0;
	}

	if ( (str.Find("her", num, SdmString::Sdm_Ignore) == 7)
	  && (str.Find("lo", num, SdmString::Sdm_Ignore) == 3)
	  && (str.Find("he", num=1, SdmString::Sdm_Ignore) == 0)
	  && (str.Find("he", num=2, SdmString::Sdm_Ignore) == 7))
	{
		cout << "PASS\tFind(): Sdm_Ignore" << endl;
	}
	else
	{
		cout << "FAIL\tFind(): Sdm_Ignore" << endl;
		status = 0;
	}

	
	if ((a < b) && (null_str < a))
	{
	    cout << "PASS\tLess Than" << endl;
	}
	else
	{
	    cout << "FAIL\tLess Than" << endl;
		status = 0;
	}

	if ((c > a) && (a > null_str))
	{
	    cout << "PASS\tGreater Than" << endl;
	}
	else
	{
	    cout << "FAIL\tGreater Than" << endl;
		status = 0;
	}

	if ((a != b) && (null_str != a))
	{
	    cout << "PASS\tNot Equal" << endl;
	}
	else
	{
	    cout << "FAIL\tNot Equal" << endl;
		status = 0;
	}

	if ((a == c) && (null_str == l))
	{
	    cout << "FAIL\tEqual" << endl;
		status = 0;
	}
	else
	{
	    cout << "PASS\tEqual" << endl;
	}

	if ((a(0) == 'a') && (a(1) == 'b') && (a(2) == 'c') && (a(3) == 'd'))
	{
		cout << "PASS\t()indexing" << endl;
	}
	else
	{
		cout << "FAIL\t()indexing" << endl;
		status = 0;
	}

	e = a;
	if (strcmp (e, "abcd") == 0)
	{
		cout << "PASS\toperator=" << endl;
	}
	else
	{
		cout << "FAIL\toperator=" << endl;
		status = 0;
	}

	a(0) = a(1);
	if (strcmp (a, "bbcd") == 0)
	{
		cout << "PASS\t()index assignment" << endl;
	}
	else
	{
		cout << "FAIL\t()index assignment" << endl;
		status = 0;
	}

	substr = str(3,7); 
	if (strcmp (substr, "lo ther") == 0)
	{
		cout << "PASS\tindexing(from,length)" << endl;
	}
	else
	{
		cout << "FAIL\tindexing(from,length)" << endl;
		status = 0;
	}

	if ((strcmp ((const char *)str, "hello there") == 0)
	  && (strcmp ((const char *)null_str, "") == 0))
	{
		cout << "PASS\tconst char * cast" << endl;
	}
	else
	{
		cout << "FAIL\tconst char * cast" << endl;
		status = 0;
	}
	
	c += d;
	g = "bunny foo foo";
	g += null_str;
	if ( (strcmp (c, "hmmzzzzzzz") == 0)
	  && (strcmp (d, "zzzzzzz") == 0)
	  && (strcmp (g, "bunny foo foo") == 0))
	{
		cout << "PASS\toperator+=" << endl;
	}
	else
	{
		cout << "FAIL\toperator+=" << endl;
		status = 0;
	}
	
	c += "elvis";
	if ( (strcmp (c, "hmmzzzzzzzelvis") == 0)
	  && (strcmp (d, "zzzzzzz") == 0))
	{
		cout << "PASS\toperator+= char *" << endl;
	}
	else
	{
		cout << "FAIL\toperator+= char *" << endl;
		status = 0;
	}

	k(k.Length()) = '\0';
	k.SetRange (7, "***bunny foo foo***");
	if (strcmp (k, ".......***bunny foo foo***....") == 0)
	{
		cout << "PASS\tSetRange" << endl;
	}
	else
	{
		cout << "FAIL\tSetRange" << endl;
		status = 0;
	}

	a = (char *) NULL;
	cout << "PASS\tassignment to NULL" << endl;

        a = "Twas brillig and the slithey toves";
        a.StripCharacter('t');
        if (a.Length() == 31 &&
            strcmp(a, "Twas brillig and he slihey oves") == 0)
	{
		cout << "PASS\tStripCharacter" << endl;
	}
	else
	{
		cout << "FAIL\tSetRange" << endl;
		status = 0;
	}
	
	x.Append(" silly world! this should not be appended", 13);
	if (x != "good bye silly world!") 
	{
		cout << "FAIL\tAppend" << endl;
		status = 0;
	}
	if (x.Length() != 21) 
	{
		cout << "FAIL\tAppend" << endl;
		status = 0;
	}
	
	y.Append("silly world! this should not be appended", 12);
	if (y != "silly world!") 
	{
		cout << "FAIL\tAppend" << endl;
		status = 0;
	}
	if (y.Length() != 12) 
	{
		cout << "FAIL\tAppend" << endl;
		status = 0;
	}
	
	if (status != 0)  
	{
		cout << "PASS\tAppend" << endl;
	}
}


static void exercise_vector ()
{
	int i;
	SdmString a("abcd"), b("efgh");
	SdmString c ("hmm");
	SdmString str;
	int pos;
	SdmString str1("hello");
	SdmString str2("there");
	SdmString str3("this");
	SdmString str4("is");
	SdmString str5("a");
	SdmString str6("test");
	SdmString str7("!");
	SdmString str8("INSERT");
	SdmVector<SdmString> strl;


	cout << endl << "SdmVector Test" << endl << endl;

	pos = strl.AddElementToList (str1);
	pos = strl.AddElementToList (str2);
	pos = strl.AddElementToList (str3);
	pos = strl.AddElementToList (str4);
	pos = strl.AddElementToList (str5);
	pos = strl.AddElementToList (str6);
	pos = strl.AddElementToList (str7);

	if (strl.ElementCount() == 7)
	{
	    cout << "PASS\tElementCount" << endl;
	}
	else
	{
	    cout << "FAIL\tElementCount" << endl;
	    status = 0;
	}

	if ((strl.ElementCount() == 7)
	  && (strl[0] == str1) && (strl[1] == str2) && (strl[2] == str3)
	  && (strl[3] == str4) && (strl[4] == str5) && (strl[5] == str6)
	  && (strl[6] == str7))
	{
	    cout << "PASS\tAddElementToList" << endl;
	}
	else
	{
	    cout << "FAIL\tAddElementToList" << endl;
	    status = 0;
	}

	strl.InsertElementAfter(str8, 3);
	if ((strl.ElementCount() == 8)
	  && (strl[4] == "INSERT"))
	{
	    cout << "PASS\tInsertElementAfter" << endl;
	}
	else
	{
	    cout << "FAIL\tInsertElementAfter" << endl;
	    status = 0;
	}

	SdmVectorIterator<SdmString> strli(&strl);
	SdmString *str_ptr;
	strli.ResetIterator();
	i=0;
	SdmBoolean bad_test = Sdm_False;
	while (str_ptr = strli.NextListElement())
	{
	  if (*str_ptr == strl[i]) 
	    i++;
	  else
	    bad_test = Sdm_True;
	}
	if (bad_test)
	{
	    cout << "FAIL\tSdmVectorIterator" << endl;
	    status = 0;
	}
	else
	{
	    cout << "PASS\tSdmVectorIterator" << endl;
	}


	strl.SetVectorSize(3);
	strl[0] = str2;
	strl[1] = str4;
	strl[2] = str6;
	if ((strl.ElementCount() == 3)
	  && (strl[0] == str2) && (strl[1] == str4) && (strl[2] == str6))
	{
	    cout << "PASS\tSetVectorSize" << endl;
	}
	else
	{
	    cout << "FAIL\tSetVectorSize" << endl;
	    status = 0;
	}
}

static void exercise_ptr_vector ()
{
	int i;
	SdmString a("abcd"), b("efgh");
	SdmString c ("hmm");
	SdmString str;
	int pos;
	SdmString *str1;
	SdmString *str2;
	SdmString *str3;
	SdmString *str4;
	SdmString *str5;
	SdmString *str6;
	SdmString *str7;
	SdmPtrVector<SdmString *> strpl;


	cout << endl << "SdmPtrVector Test" << endl << endl;

	strpl.SetPointerDeleteFlag();

	str1 = new SdmString ("hello");
	str2 = new SdmString ("there");
	str3 = new SdmString ("this");
	str4 = new SdmString ("is");
	str5 = new SdmString ("a");
	str6 = new SdmString ("test");
	str7 = new SdmString ("!");

	pos = strpl.AddElementToList (str1);
	pos = strpl.AddElementToList (str2);
	pos = strpl.AddElementToList (str3);
	pos = strpl.AddElementToList (str4);
	pos = strpl.AddElementToList (str5);
	pos = strpl.AddElementToList (str6);
	pos = strpl.AddElementToList (str7);

	if (strpl.ElementCount() == 7)
	{
	    cout << "PASS\tElementCount" << endl;
	}
	else
	{
	    cout << "FAIL\tElementCount" << endl;
	    status = 0;
	}

	if ((strpl.ElementCount() == 7)
	  && (*strpl[0] == *str1) && (*strpl[1] == *str2) &&(*strpl[2] == *str3)
	  && (*strpl[3] == *str4) && (*strpl[4] == *str5) &&(*strpl[5] == *str6)
	  && (*strpl[6] == *str7))
	{
	    cout << "PASS\tAddElementToList" << endl;
	}
	else
	{
	    cout << "FAIL\tAddElementToList" << endl;
	    status = 0;
	}

	SdmPtrVector<SdmString *> strpl2(strpl);
	strpl2.ClearPointerDeleteFlag();
	if ((strpl2.ElementCount() == 7)
	  && (*strpl2[0] == *str1) && (*strpl2[1] == *str2) 
	  && (*strpl2[2] == *str3) && (*strpl2[3] == *str4) 
	  && (*strpl2[4] == *str5) && (*strpl2[5] == *str6)
	  && (*strpl2[6] == *str7))
	{
	    cout << "PASS\tCopy Constructor" << endl;
	}
	else
	{
	    cout << "FAIL\tCopy Constructor" << endl;
	    status = 0;
	}


	SdmPtrVector<SdmString *> strpl3;
	strpl3 = strpl;
	strpl3.ClearPointerDeleteFlag();
	if ((strpl3.ElementCount() == 7)
	  && (*strpl3[0] == *str1) && (*strpl3[1] == *str2) 
	  && (*strpl3[2] == *str3) && (*strpl3[3] == *str4) 
	  && (*strpl3[4] == *str5) && (*strpl3[5] == *str6)
	  && (*strpl3[6] == *str7))
	{
	    cout << "PASS\tCopy Constructor" << endl;
	}
	else
	{
	    cout << "FAIL\tCopy Constructor" << endl;
	    status = 0;
	}

	SdmVectorIterator<SdmString *> strpli(&strpl);
	SdmString **str_ptr;
	strpli.ResetIterator();
	i = 0;
	SdmBoolean bad_test = Sdm_False;
	while (str_ptr = strpli.NextListElement())
	{
	  if (**str_ptr != *strpl[i]) 
	    bad_test = Sdm_True;
	  i++;
	}
	if (bad_test)
	{
	    cout << "FAIL\tSdmVectorIterator" << endl;
	    status = 0;
	}
	else
	{
	    cout << "PASS\tSdmVectorIterator" << endl;
	}

}

static void exercise_sort_vector ()
{
	int i;
	SdmString a("abcd"), b("efgh");
	SdmString c ("hmm");
	SdmString str;
	int pos;
	SdmString str1("hello");
	SdmString str2("there");
	SdmString str3("this");
	SdmString str4("is");
	SdmString str5("a");
	SdmString str6("test");
	SdmString str7("!");
	SdmSortVector<SdmString> strl;


	cout << endl << "SdmSortVector Test" << endl << endl;

	pos = strl.AddElementToList (str1);
	pos = strl.AddElementToList (str2);
	pos = strl.AddElementToList (str3);
	pos = strl.AddElementToList (str4);
	pos = strl.AddElementToList (str5);
	pos = strl.AddElementToList (str6);
	pos = strl.AddElementToList (str7);

	if (strl.ElementCount() == 7)
	{
	    cout << "PASS\tElementCount" << endl;
	}
	else
	{
	    cout << "FAIL\tElementCount" << endl;
	    status = 0;
	}

	if ((strl.ElementCount() == 7)
	  && (strl[0] == str7) && (strl[1] == str5) && (strl[2] == str1)
	  && (strl[3] == str4) && (strl[4] == str6) && (strl[5] == str2)
	  && (strl[6] == str3))
	{
	    cout << "PASS\tAddElementToList" << endl;
	}
	else
	{
	    cout << "FAIL\tAddElementToList" << endl;
	    status = 0;
	}

	
	SdmSortVector<SdmString> strl2(strl);
	if ((strl2.ElementCount() == 7)
	  && (strl2[0] == str7) && (strl2[1] == str5) && (strl2[2] == str1)
	  && (strl2[3] == str4) && (strl2[4] == str6) && (strl2[5] == str2)
	  && (strl2[6] == str3))
	{
	    cout << "PASS\tCopy Constructor" << endl;
	}
	else
	{
	    cout << "FAIL\tCopy Constructor" << endl;
	    status = 0;
	}


	SdmSortVector<SdmString> strl3;
	strl3 = strl;
	if ((strl3.ElementCount() == 7)
	  && (strl3[0] == str7) && (strl3[1] == str5) && (strl3[2] == str1)
	  && (strl3[3] == str4) && (strl3[4] == str6) && (strl3[5] == str2)
	  && (strl3[6] == str3))
	{
	    cout << "PASS\tOperator=" << endl;
	}
	else
	{
	    cout << "FAIL\tOperator=" << endl;
	    status = 0;
	}

	strl.ClearAllElements();
	if (strl.ElementCount() == 0)
	{
	    cout << "PASS\tClearAllElements" << endl;
	}
	else
	{
	    cout << "FAIL\tClearAllElements" << endl;
	    status = 0;
	}

}

static void exercise_ptr_sort_vector ()
{
	int i;
	SdmString a("abcd"), b("efgh");
	SdmString c ("hmm");
	SdmString str;
	int pos;
	SdmString *str1;
	SdmString *str2;
	SdmString *str3;
	SdmString *str4;
	SdmString *str5;
	SdmString *str6;
	SdmString *str7;
	SdmPtrSortVector<SdmString *> strpl;


	cout << endl << "SdmPtrSortVector Test" << endl << endl;

	str1 = new SdmString ("hello");
	str2 = new SdmString ("there");
	str3 = new SdmString ("this");
	str4 = new SdmString ("is");
	str5 = new SdmString ("a");
	str6 = new SdmString ("test");
	str7 = new SdmString ("!");

	pos = strpl.AddElementToList (str1);
	pos = strpl.AddElementToList (str2);
	pos = strpl.AddElementToList (str3);
	pos = strpl.AddElementToList (str4);
	pos = strpl.AddElementToList (str5);
	pos = strpl.AddElementToList (str6);
	pos = strpl.AddElementToList (str7);

	if (strpl.ElementCount() == 7)
	{
	    cout << "PASS\tElementCount" << endl;
	}
	else
	{
	    cout << "FAIL\tElementCount" << endl;
	    status = 0;
	}

	if ((strpl.ElementCount() == 7)
	  && (*strpl[0] == *str7) && (*strpl[1] == *str5) 
	  && (*strpl[2] == *str1) && (*strpl[3] == *str4) 
	  && (*strpl[4] == *str6) && (*strpl[5] == *str2)
	  && (*strpl[6] == *str3))
	{
	    cout << "PASS\tAddElementToList" << endl;
	}
	else
	{
	    cout << "FAIL\tAddElementToList" << endl;
	    status = 0;
	}

	SdmPtrSortVector<SdmString *> strpl2(strpl);
	if ((strpl2.ElementCount() == 7)
	  && (*strpl2[0] == *str7) && (*strpl2[1] == *str5) 
	  && (*strpl2[2] == *str1) && (*strpl2[3] == *str4) 
	  && (*strpl2[4] == *str6) && (*strpl2[5] == *str2)
	  && (*strpl2[6] == *str3))
	{
	    cout << "PASS\tCopy Constructor" << endl;
	}
	else
	{
	    cout << "FAIL\tCopy Constructor" << endl;
	    status = 0;
	}


	SdmPtrSortVector<SdmString *> strpl3;
	strpl3 = strpl;
	if ((strpl3.ElementCount() == 7)
	  && (*strpl3[0] == *str7) && (*strpl3[1] == *str5) 
	  && (*strpl3[2] == *str1) && (*strpl3[3] == *str4) 
	  && (*strpl3[4] == *str6) && (*strpl3[5] == *str2)
	  && (*strpl3[6] == *str3))
	{
	    cout << "PASS\tOperator=" << endl;
	}
	else
	{
	    cout << "FAIL\tOperator=" << endl;
	    status = 0;
	}

	strpl.SetPointerDeleteFlag();
}


static void exercise_dlist ()
{
	SdmStr a("abcd"), b("efgh");
	SdmStr c ("hmm");
	SdmStr str;
	SdmStr str1("hello");
	SdmStr str2("there");
	SdmStr str3("this");
	SdmStr str4("is");
	SdmStr str5("a");
	SdmStr str6("test");
	SdmStr str7("!");
	SdmDoubleLinkedList dll;
	SdmDoubleLinkedListIterator dlli(&dll);
	void *ptr;
	int pos;
	int i;
	SdmStr *strl1, *strl2, *strl3, *strl4, *strl5, *strl6, *strl7;
	SdmPtrVector<SdmStr *> strpl;
	SdmBoolean bad_test = Sdm_False;


	cout << endl << "Doubly Linked List Test" << endl << endl;


	strpl.SetPointerDeleteFlag();

	strl1 = new SdmStr ("hello");
	strl2 = new SdmStr ("there");
	strl3 = new SdmStr ("this");
	strl4 = new SdmStr ("is");
	strl5 = new SdmStr ("a");
	strl6 = new SdmStr ("test");
	strl7 = new SdmStr ("!");

	pos = strpl.AddElementToList (strl1);
	pos = strpl.AddElementToList (strl2);
	pos = strpl.AddElementToList (strl3);
	pos = strpl.AddElementToList (strl4);
	pos = strpl.AddElementToList (strl5);
	pos = strpl.AddElementToList (strl6);
	pos = strpl.AddElementToList (strl7);

	dll.AddElementToList (&str1);
	dll.AddElementToList (&str2);
	dll.AddElementToList (&str3);
	dll.AddElementToList (&str4);
	dll.AddElementToList (&str5);
	dll.AddElementToList (&str6);
	dll.AddElementToList (&str7);
	
	if (dll.ElementCount() == 7)
	{
	    cout << "PASS\tElementCount" << endl;
	}
	else
	{
	    cout << "FAIL\tElementCount" << endl;
	    status = 0;
	}

	dlli.ResetIterator();
	i = 0;
	while (ptr = dlli.NextListElement())
	{
	  if ((*(SdmStr *)ptr) == (*strpl[i])) 
	    i++;
	  else
	    bad_test = Sdm_True;
	}
	if (bad_test)
	{
	    cout << "FAIL\tIterate Forwards" << endl;
	    status = 0;
	}
	else
	{
	    cout << "PASS\tIterate Forwards" << endl;
	}


	dlli.ResetIterator();
	i = dll.ElementCount() - 1;
	while (ptr = dlli.PrevListElement())
	{
	  if (*(SdmStr *)ptr != *strpl[i]) 
	    bad_test = Sdm_True;
	  i--;
	}
	if (bad_test)
	{
	    cout << "FAIL\tIterate Backwards" << endl;
	    status = 0;
	}
	else
	{
	    cout << "PASS\tIterate Backwards" << endl;
	}

	pos = dll.FindElement (&str3);
	if (*(SdmStr *)(dll[pos]) == str3)
	{
	    cout << "PASS\tFindElement" << endl;
	}
	else
	{
	    cout << "FAIL\tFindElement" << endl;
	    status = 0;
	}

	dll.RemoveElementFromList(&str3);
	dll.RemoveElementFromList(&str5);
	if ((dll.ElementCount() == 5)
	  && (*((SdmStr *)(dll[0])) == str1)
	  && (*((SdmStr *)(dll[1])) == str2)
	  && (*((SdmStr *)(dll[2])) == str4)
	  && (*((SdmStr *)(dll[3])) == str6)
	  && (*((SdmStr *)(dll[4])) == str7))
	{
	    cout << "PASS\tRemoveElementFromList" << endl;
	}
	else
	{
	    cout << "FAIL\tRemoveElementFromList" << endl;
	    status = 0;
	}

  // note: don't call ClearAndDestroyAllElements because the
  // elements in dll are from the stack.
	dll.ClearAllElements();  
	if (dll.ElementCount() == 0)
	{
	    cout << "PASS\tClearAndDestroyAllElements" << endl;
	}
	else
	{
	    cout << "FAIL\tClearAndDestroyAllElements" << endl;
	    status = 0;
	}
}

static void exercise_time ()
{
	cout << endl << "Test SdmTime" << endl << endl;

	SdmTime		t1;
	sleep (5);
	SdmTime		t2;
	SdmTime		t3;

	t3 = t1;
	if (t1 == t3)
	{
	    cout << "PASS\tEqual" << endl;
	}
	else
	{
	    cout << "FAIL\tEqual" << endl;
	    status = 0;
	}

	if (t1 < t2)
	{
	    cout << "PASS\tLess Than" << endl;
	}
	else
	{
	    cout << "FAIL\tLess Than" << endl;
	    status = 0;
	}

	if (t1 > t2)
	{
	    cout << "FAIL\tGreater Than" << endl;
	    status = 0;
	}
	else
	{
	    cout << "PASS\tGreater Than" << endl;
	}


	t1 = t2;
	if (t1 == t2)
	{
	    cout << "PASS\tOperator=" << endl;
	}
	else
	{
	    cout << "FAIL\tOperator=" << endl;
	    status = 0;
	}

}

static const NumThreads = 100;
static SdmBoolean corruption = Sdm_False;
static int counter;

static void *
proc(void *client_data)
{
  SdmRecursiveMutexEntry::Data *mutex_data =
    (SdmRecursiveMutexEntry::Data *)client_data;
  static thread_t critical_data;
  unsigned seed = 0xdeadbeef;

  // Comment the following to potentially see the critical data corrupted
  SdmRecursiveMutexEntry mutex(*mutex_data);

  critical_data = thr_self();

  // Do a little work - gives a greater window of exposure for other threads
  // to attempt to reach this critical section.
  for (counter = 0; counter < 10000; counter++);

  // Recurse occasionally - this thread should be allowed to proceed
  if (rand() % 10 == 5) {
    proc(mutex_data);
  }

  if (critical_data != thr_self()) {
    corruption = Sdm_True;
  }

  return 0;
}

static void exercise_recursive_mutex ()
{
  // Use the commented constructor below to potentially see the concurrent
  // thread access warning
  SdmRecursiveMutexEntry::Data mutex_data(Sdm_True);
  // SdmRecursiveMutexEntry::Data mutex_data;
  thread_t tid;

  cout << endl << "SdmRecursiveMutex Test" << endl << endl;

  cout << "main thread " << thr_self() << endl;

  srand((unsigned)time(NULL));

  thr_setconcurrency(100);

  for (int i = 0; i < NumThreads; i++) {
    thr_create(NULL, 0, proc, &mutex_data, 0, &tid);
  }
  while (thr_join(NULL, NULL, NULL) == 0)
    ;

  if (corruption)
    cout << endl << "FAIL\tcritical data corrupted" << endl << endl;
  else
    cout << endl << "PASS\tcritical data intact" << endl << endl;

  cout << "main thread (exit)" << endl;
}
