/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * Implementation of the DOM nsIDOMRange object.
 */

#ifndef nsRange_h___
#define nsRange_h___

#include "nsIDOMRange.h"
#include "nsCOMPtr.h"
#include "nsINode.h"
#include "nsIDocument.h"
#include "nsIDOMNode.h"
#include "prmon.h"
#include "nsStubMutationObserver.h"
#include "nsWrapperCache.h"
#include "mozilla/Attributes.h"

class nsClientRect;
class nsClientRectList;
class nsIDOMDocumentFragment;

namespace mozilla {
class ErrorResult;
namespace dom {
class DocumentFragment;
}
}

class nsRange MOZ_FINAL : public nsIDOMRange,
                          public nsStubMutationObserver,
                          public nsWrapperCache
{
  typedef mozilla::ErrorResult ErrorResult;

public:
  nsRange(nsINode* aNode)
    : mRoot(nullptr)
    , mStartOffset(0)
    , mEndOffset(0)
    , mIsPositioned(false)
    , mIsDetached(false)
    , mMaySpanAnonymousSubtrees(false)
    , mInSelection(false)
    , mStartOffsetWasIncremented(false)
    , mEndOffsetWasIncremented(false)
#ifdef DEBUG
    , mAssertNextInsertOrAppendIndex(-1)
    , mAssertNextInsertOrAppendNode(nullptr)
#endif
  {
    SetIsDOMBinding();
    MOZ_ASSERT(aNode, "range isn't in a document!");
    mOwner = aNode->OwnerDoc();
  }
  virtual ~nsRange();

  static nsresult CreateRange(nsIDOMNode* aStartParent, int32_t aStartOffset,
                              nsIDOMNode* aEndParent, int32_t aEndOffset,
                              nsRange** aRange);
  static nsresult CreateRange(nsIDOMNode* aStartParent, int32_t aStartOffset,
                              nsIDOMNode* aEndParent, int32_t aEndOffset,
                              nsIDOMRange** aRange);

  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS_AMBIGUOUS(nsRange, nsIDOMRange)

  // nsIDOMRange interface
  NS_DECL_NSIDOMRANGE
  
  nsINode* GetRoot() const
  {
    return mRoot;
  }

  nsINode* GetStartParent() const
  {
    return mStartParent;
  }

  nsINode* GetEndParent() const
  {
    return mEndParent;
  }

  int32_t StartOffset() const
  {
    return mStartOffset;
  }

  int32_t EndOffset() const
  {
    return mEndOffset;
  }
  
  bool IsPositioned() const
  {
    return mIsPositioned;
  }

  void SetMaySpanAnonymousSubtrees(bool aMaySpanAnonymousSubtrees)
  {
    mMaySpanAnonymousSubtrees = aMaySpanAnonymousSubtrees;
  }
  
  /**
   * Return true iff this range is part of at least one Selection object
   * and isn't detached.
   */
  bool IsInSelection() const
  {
    return mInSelection;
  }

  /**
   * Called when the range is added/removed from a Selection.
   */
  void SetInSelection(bool aInSelection)
  {
    if (mInSelection == aInSelection) {
      return;
    }
    mInSelection = aInSelection;
    nsINode* commonAncestor = GetCommonAncestor();
    NS_ASSERTION(commonAncestor, "unexpected disconnected nodes");
    if (mInSelection) {
      RegisterCommonAncestor(commonAncestor);
    } else {
      UnregisterCommonAncestor(commonAncestor);
    }
  }

  nsINode* GetCommonAncestor() const;
  void Reset();
  nsresult SetStart(nsINode* aParent, int32_t aOffset);
  nsresult SetEnd(nsINode* aParent, int32_t aOffset);
  already_AddRefed<nsRange> CloneRange() const;

  nsresult Set(nsINode* aStartParent, int32_t aStartOffset,
               nsINode* aEndParent, int32_t aEndOffset)
  {
    // If this starts being hot, we may be able to optimize this a bit,
    // but for now just set start and end separately.
    nsresult rv = SetStart(aStartParent, aStartOffset);
    NS_ENSURE_SUCCESS(rv, rv);

    return SetEnd(aEndParent, aEndOffset);
  }

  NS_IMETHOD GetUsedFontFaces(nsIDOMFontFaceList** aResult);

  // nsIMutationObserver methods
  NS_DECL_NSIMUTATIONOBSERVER_CHARACTERDATACHANGED
  NS_DECL_NSIMUTATIONOBSERVER_CONTENTINSERTED
  NS_DECL_NSIMUTATIONOBSERVER_CONTENTREMOVED
  NS_DECL_NSIMUTATIONOBSERVER_PARENTCHAINCHANGED
  NS_DECL_NSIMUTATIONOBSERVER_CONTENTAPPENDED

  // WebIDL
  bool Collapsed() const
  {
    return mIsPositioned && mStartParent == mEndParent &&
           mStartOffset == mEndOffset;
  }
  already_AddRefed<mozilla::dom::DocumentFragment>
  CreateContextualFragment(const nsAString& aString, ErrorResult& aError);
  already_AddRefed<mozilla::dom::DocumentFragment>
  CloneContents(ErrorResult& aErr);
  int16_t CompareBoundaryPoints(uint16_t aHow, nsRange& aOther,
                                ErrorResult& aErr);
  int16_t ComparePoint(nsINode& aParent, uint32_t aOffset, ErrorResult& aErr);
  void DeleteContents(ErrorResult& aRv);
  already_AddRefed<mozilla::dom::DocumentFragment>
    ExtractContents(ErrorResult& aErr);
  nsINode* GetCommonAncestorContainer(ErrorResult& aRv) const;
  nsINode* GetStartContainer(ErrorResult& aRv) const;
  uint32_t GetStartOffset(ErrorResult& aRv) const;
  nsINode* GetEndContainer(ErrorResult& aRv) const;
  uint32_t GetEndOffset(ErrorResult& aRv) const;
  void InsertNode(nsINode& aNode, ErrorResult& aErr);
  bool IntersectsNode(nsINode& aNode, ErrorResult& aRv);
  bool IsPointInRange(nsINode& aParent, uint32_t aOffset, ErrorResult& aErr);
  void SelectNode(nsINode& aNode, ErrorResult& aErr);
  void SelectNodeContents(nsINode& aNode, ErrorResult& aErr);
  void SetEnd(nsINode& aNode, uint32_t aOffset, ErrorResult& aErr);
  void SetEndAfter(nsINode& aNode, ErrorResult& aErr);
  void SetEndBefore(nsINode& aNode, ErrorResult& aErr);
  void SetStart(nsINode& aNode, uint32_t aOffset, ErrorResult& aErr);
  void SetStartAfter(nsINode& aNode, ErrorResult& aErr);
  void SetStartBefore(nsINode& aNode, ErrorResult& aErr);
  void SurroundContents(nsINode& aNode, ErrorResult& aErr);
  already_AddRefed<nsClientRect> GetBoundingClientRect();
  already_AddRefed<nsClientRectList> GetClientRects();

  nsINode* GetParentObject() const { return mOwner; }
  virtual JSObject* WrapObject(JSContext* cx, JSObject* scope) MOZ_OVERRIDE MOZ_FINAL;

private:
  // no copy's or assigns
  nsRange(const nsRange&);
  nsRange& operator=(const nsRange&);

  /**
   * Cut or delete the range's contents.
   *
   * @param aFragment nsIDOMDocumentFragment containing the nodes.
   *                  May be null to indicate the caller doesn't want a fragment.
   */
  nsresult CutContents(mozilla::dom::DocumentFragment** frag);

  static nsresult CloneParentsBetween(nsIDOMNode *aAncestor,
                                      nsIDOMNode *aNode,
                                      nsIDOMNode **aClosestAncestor,
                                      nsIDOMNode **aFarthestAncestor);

public:
/******************************************************************************
 *  Utility routine to detect if a content node starts before a range and/or 
 *  ends after a range.  If neither it is contained inside the range.
 *  
 *  XXX - callers responsibility to ensure node in same doc as range!
 *
 *****************************************************************************/
  static nsresult CompareNodeToRange(nsINode* aNode, nsRange* aRange,
                                     bool *outNodeBefore,
                                     bool *outNodeAfter);

  static bool IsNodeSelected(nsINode* aNode, uint32_t aStartOffset,
                             uint32_t aEndOffset);

  typedef nsTHashtable<nsPtrHashKey<nsRange> > RangeHashTable;
protected:
  void RegisterCommonAncestor(nsINode* aNode);
  void UnregisterCommonAncestor(nsINode* aNode);
  nsINode* IsValidBoundary(nsINode* aNode);

  // CharacterDataChanged set aNotInsertedYet to true to disable an assertion
  // and suppress re-registering a range common ancestor node since
  // the new text node of a splitText hasn't been inserted yet.
  // CharacterDataChanged does the re-registering when needed.
  void DoSetRange(nsINode* aStartN, int32_t aStartOffset,
                  nsINode* aEndN, int32_t aEndOffset,
                  nsINode* aRoot, bool aNotInsertedYet = false);

  /**
   * For a range for which IsInSelection() is true, return the common
   * ancestor for the range.  This method uses the selection bits and
   * nsGkAtoms::range property on the nodes to quickly find the ancestor.
   * That is, it's a faster version of GetCommonAncestor that only works
   * for ranges in a Selection.  The method will assert and the behavior
   * is undefined if called on a range where IsInSelection() is false.
   */
  nsINode* GetRegisteredCommonAncestor();

  struct NS_STACK_CLASS AutoInvalidateSelection
  {
    AutoInvalidateSelection(nsRange* aRange) : mRange(aRange)
    {
#ifdef DEBUG
      mWasInSelection = mRange->IsInSelection();
#endif
      if (!mRange->IsInSelection() || mIsNested) {
        return;
      }
      mIsNested = true;
      mCommonAncestor = mRange->GetRegisteredCommonAncestor();
    }
    ~AutoInvalidateSelection();
    nsRange* mRange;
    nsRefPtr<nsINode> mCommonAncestor;
#ifdef DEBUG
    bool mWasInSelection;
#endif
    static bool mIsNested;
  };

  nsCOMPtr<nsIDocument> mOwner;
  nsCOMPtr<nsINode> mRoot;
  nsCOMPtr<nsINode> mStartParent;
  nsCOMPtr<nsINode> mEndParent;
  int32_t mStartOffset;
  int32_t mEndOffset;

  bool mIsPositioned;
  bool mIsDetached;
  bool mMaySpanAnonymousSubtrees;
  bool mInSelection;
  bool mStartOffsetWasIncremented;
  bool mEndOffsetWasIncremented;
#ifdef DEBUG
  int32_t  mAssertNextInsertOrAppendIndex;
  nsINode* mAssertNextInsertOrAppendNode;
#endif
};

inline nsISupports*
ToCanonicalSupports(nsRange* aRange)
{
  return static_cast<nsIDOMRange*>(aRange);
}

inline nsISupports*
ToSupports(nsRange* aRange)
{
  return static_cast<nsIDOMRange*>(aRange);
}

#endif /* nsRange_h___ */
